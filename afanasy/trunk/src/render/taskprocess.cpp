#include "taskprocess.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mclistenaddress.h"
#include "../libafanasy/msgclasses/mctaskoutput.h"
#include "../libafanasy/msgclasses/mctaskup.h"

#include "renderhost.h"
#include "parserhost.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

extern void (*fp_setupChildProcess)( void);

void setupChildProcess( void)
{
//    printf("This is child process!\n");
    if( setsid() == -1) AFERRPE("setsid")
    int nicenew = nice( af::Environment::getRenderNice());
    if( nicenew == -1) AFERRPE("nice")
}

// Taken from:
// http://www.kegel.com/dkftpbench/nonblocking.html
int setNonblocking(int fd)
{
    int flags;

    /* If they have O_NONBLOCK, use the Posix way to do it */
#if defined(O_NONBLOCK)
    /* Fixme: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    /* Otherwise, use the old way of doing it */
    flags = 1;
    return ioctl(fd, FIOBIO, &flags);
#endif
}

TaskProcess::TaskProcess( af::TaskExec * i_taskExec):
    m_taskexec( i_taskExec),
    m_service( *i_taskExec),
    m_parser( NULL),
    m_update_status( af::TaskExec::UPPercent),
    m_stop_time( 0),
    m_zombie( false)
{
    std::string command = m_service.getCommand();
    std::string wdir    = m_service.getWDir();

    m_parser = new ParserHost( m_taskexec->getParserType(), m_taskexec->getFramesNum());

    // Process task working directory:
    if( wdir.size())
    {
#ifdef WINNT
        if( wdir.find('/') == 0 )
        {
            AFERRAR("Working directory starts with '/':\n%s\nMay be it is not translated from UNIX?", wdir.c_str())
            wdir.clear();
        }
        else if( wdir.find('\\') == 0)
        {
            AFERRAR("Working directory starts with '\\':\n%s\nUNC path can't be current. May be incorrect translation?", wdir.c_str())
            wdir.clear();
        }
        else
#endif
        if( false == af::pathIsFolder( wdir))
        {
            AFERRAR("Working directory does not exists:\n%s", wdir.c_str())
            wdir.clear();
        }
    }

    if( af::Environment::isVerboseMode()) printf("%s\n", command.c_str());

    fp_setupChildProcess = setupChildProcess;

    int flags = 0;
#ifdef WINNT
    flags = CREATE_SUSPENDED;
#endif
    m_pid = af::launchProgram( command, wdir, &m_io_input, &m_io_output, &m_io_outerr, flags);

    if( m_pid <= 0 )
    {
        AFERROR("Failed to start a process")
        m_taskexec->stdOut( true);
        m_pid = 0;
        m_update_status = af::TaskExec::UPFailedToStart;
        sendTaskSate();
        return;
    }

#ifdef WINNT
    process_info = m_pid;
    SetPriorityClass( process_info->hProcess, BELOW_NORMAL_PRIORITY_CLASS);
    if( AssignProcessToJobObject( hJob, process_info->hProcess) == false)
        AFERROR("AssignProcessToJobObject failed.\n")
    ResumeThread( process_info->hThread);
#endif

    setbuf( m_io_output, m_filebuffer_out);
    setbuf( m_io_outerr, m_filebuffer_err);

    setNonblocking( fileno( m_io_input));
    setNonblocking( fileno( m_io_output));
    setNonblocking( fileno( m_io_outerr));

    printf("Started PID=%d: ",m_pid);
    m_taskexec->stdOut( af::Environment::isVerboseMode());
}

TaskProcess::~TaskProcess()
{
    m_update_status = 0;

    killProcess();

    fclose( m_io_input);
    fclose( m_io_output);
    fclose( m_io_outerr);

#ifdef AFOUTPUT
    printf(" ~ TaskProcess(): ");
    m_taskexec->stdOut();
#endif

    if( m_taskexec != NULL  ) delete m_taskexec;
    if( m_parser   != NULL  ) delete m_parser;
}

void TaskProcess::refresh()
{
    if( m_stop_time )
    {
        if( m_pid == 0 ) m_zombie = true;
        else if( time( NULL) - m_stop_time > AFRENDER::TERMINATEWAITKILL ) killProcess();
    }

    if( m_pid == 0 )
    {
        static int dead_cycle = 0;
        dead_cycle ++;
//        if(( dead_cycle % 10 ) == 0 )
            sendTaskSate();
        printf("Dead Cycle:"); m_taskexec->stdOut();
        return;
    }

    int status;
    pid_t pid = waitpid( m_pid, &status, WNOHANG);

    if( pid == 0 )
    {
        readProcess();
    }
    else if( pid == m_pid )
    {
        processFinished( status);
    }
    else if( pid == -1 )
    {
        AFERRPE("TaskProcess::refresh(): waitpid: ")
    }

    sendTaskSate();
}

void TaskProcess::readProcess()
{
    std::string output;

    for( int i = 0; i < 2; i++)
    {
        FILE * pFile = m_io_output;
        if( i )
            pFile = m_io_outerr;

        fflush( pFile);

        if( feof( pFile))
            continue;

        int readsize = fread( m_readbuffer, 1, m_readbuffer_size, pFile );
        if( readsize <= 0 )
        {
            if( errno == EAGAIN )
            {
                readsize = fread( m_readbuffer, 1, m_readbuffer_size, pFile );
                if( readsize <= 0 )
                    continue;
            }
            else
                continue;
        }

        output += std::string( m_readbuffer, readsize);
    }

    if( output.size() == 0 ) return;

    m_parser->read( output);

    if( m_taskexec->getListenAddressesNum())
    {
        af::MCTaskOutput mctaskoutput( RenderHost::getName(), m_taskexec->getJobId(), m_taskexec->getBlockNum(), m_taskexec->getTaskNum(), output.size(), output.data());
        af::Msg * msg = new af::Msg( af::Msg::TTaskOutput, &mctaskoutput);
        msg->setAddresses( *m_taskexec->getListenAddresses());
        RenderHost::dispatchMessage( msg);
    }

    if( m_parser->hasWarning() && (m_update_status != af::TaskExec::UPWarning) &&
           (m_update_status != af::TaskExec::UPFinishedParserError))
    {
        printf("Warning: Parser notification.\n");
        m_update_status = af::TaskExec::UPWarning;
    }
    if( m_parser->hasError() && ( m_stop_time == 0 ))
    {
        printf("Error: Bad result from m_parser. Stopping task.\n");
        m_update_status = af::TaskExec::UPFinishedParserError;
        stop();
    }
}

void TaskProcess::sendTaskSate()
{
    if( m_update_status == 0 ) return;

    int    type = af::Msg::TTaskUpdatePercent;
    bool   toRecieve = false;
    char * stdout_data = NULL;
    int    stdout_size = 0;

    if(( m_update_status != af::TaskExec::UPPercent ) &&
        ( m_update_status != af::TaskExec::UPWarning ))
    {
        type = af::Msg::TTaskUpdateState;
        toRecieve = true;
        if( m_parser) stdout_data = m_parser->getData( &stdout_size);
    }

    int percent        = 0;
    int frame          = 0;
    int percentframe   = 0;

    if( m_parser)
    {
        percent        = m_parser->getPercent();
        frame          = m_parser->getFrame();
        percentframe   = m_parser->getPercentFrame();
    }

    af::MCTaskUp taskup(
                     RenderHost::getId(),

                     m_taskexec->getJobId(),
                     m_taskexec->getBlockNum(),
                     m_taskexec->getTaskNum(),
                     m_taskexec->getNumber(),

                     m_update_status,
                     percent,
                     frame,
                     percentframe,

                     stdout_size,
                     stdout_data
                  );

    af::Msg * msg = new af::Msg( type, &taskup);
    if( toRecieve) msg->setReceiving();

//    printf("TaskProcess::sendTaskSate:\n");msg->stdOut();printf("\n");

    RenderHost::dispatchMessage( msg);
}

void TaskProcess::processFinished( int i_exitCode)
{
    printf("Finished PID=%d: Exit Code=%d\n", m_pid, i_exitCode);

    m_pid = 0;
    if( m_update_status == 0 ) return;

    readProcess();

    if( i_exitCode != 0)
    {
        if(( m_stop_time != 0 ) || WIFSIGNALED( i_exitCode))
        {
            printf("Task terminated/killed by signal: '%s'.\n", strsignal( WTERMSIG( i_exitCode)));
            if( m_update_status != af::TaskExec::UPFinishedParserError )
                m_update_status  = af::TaskExec::UPFinishedKilled;
        }
        else
        {
            m_update_status = af::TaskExec::UPFinishedError;
        }
    }
    else if( m_parser->isBadResult())
    {
        m_update_status = af::TaskExec::UPFinishedParserBadResult;
        AFINFO("Bad result from parser.")
    }
    else
    {
        m_update_status = af::TaskExec::UPFinishedSuccess;
    }
}

void TaskProcess::stop()
{
    // Store the time when task was asked to be stopped (was asked first time)
    if( m_stop_time == 0 ) m_stop_time = time(NULL);

    // Return if task is not running
    if( m_pid == 0 ) return;

    // Trying to terminate() first, and only if no response after some time, then perform kill()
#ifdef UNIX
    killpg( getpgid( m_pid), SIGTERM);
#else
    CloseHandle( hJob );
#endif
}

void TaskProcess::killProcess()
{
    if( m_pid == 0 ) return;
    printf("KILLING NOT TERMINATED TASK.\n");
#ifdef UNIX
    killpg( getpgid( m_pid), SIGKILL);
#else
    CloseHandle( hJob );
#endif
}

void TaskProcess::getOutput( af::Msg * o_msg) const
{
    if( m_parser != NULL)
    {
        int size;
        char *data = m_parser->getData( &size);
        if( size > 0)
        {
            o_msg->setData( size, data);
        }
        else
        {
            o_msg->setString("Render: Silence...");
        }
    }
    else
    {
        o_msg->setString("Render: Parser is NULL.");
    }
}
