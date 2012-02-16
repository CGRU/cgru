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
/*
void thread_entry_point( void *i_parameter )
{
   TaskProcess * taskprocess = (TaskProcess *) i_parameter;
   taskprocess->listen();
}
*/
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

    m_parser = new ParserHost( m_taskexec->getParserType(), m_taskexec->getFramesNum());//, "AF_PROGRESS %d");

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

    printf("\nStarting:\n ");
    m_taskexec->stdOut( false);
    if( af::Environment::isVerboseMode()) printf("%s\n", command.c_str());

//    m_pid = af::launchProgram( command, wdir);
//    m_pid = af::launchProgram( command, wdir, NULL, &stdout, &stderr);
    m_pid = af::launchProgram( command, wdir, &m_io_input, &m_io_output, &m_io_outerr);

//    m_thread->Start( thread_entry_point, this);

    if( m_pid <= 0 )
    {
        AFERROR("Failed to start a process")
        m_pid = 0;
        return;
    }

    setNonblocking( fileno( m_io_input));
    setNonblocking( fileno( m_io_output));
    setNonblocking( fileno( m_io_outerr));

    setbuf( m_io_output, m_filebuffer_out);
    setbuf( m_io_outerr, m_filebuffer_err);
}

TaskProcess::~TaskProcess()
{
   m_update_status = 0;
   killProcess();

   fclose( m_io_input);
   fclose( m_io_output);
   fclose( m_io_outerr);

   if( m_taskexec    != NULL  ) delete m_taskexec;
   if( m_parser  != NULL  ) delete m_parser;

   AFINFO("TaskProcess:~TaskProcess()")
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

    printf("TaskProcess::sendTaskSate:\n");msg->stdOut();printf("\n");

    RenderHost::dispatchMessage( msg);
}

void TaskProcess::refresh()
{
    printf("TaskProcess::refresh():\n");

    if( m_pid == 0 )
    {
        return;
    }

    int status;
    pid_t pid = waitpid( m_pid, &status, WNOHANG);

    if( pid == 0 )
    {
        readProcess();
        sendTaskSate();
    }
    else if( pid == m_pid )
    {
        processFinished( status);
    }
    else if( pid == -1 )
    {
        AFERRPE("TaskProcess::refresh(): waitpid: ")
    }

    if( m_stop_time )
    {
        if( m_pid == 0 ) m_zombie = true;
        else if( time( NULL) - m_stop_time > AFRENDER::TERMINATEWAITKILL ) killProcess();
    }
}

void TaskProcess::processFinished( int exitCode)
{
    m_pid = 0;
    if( m_update_status == 0 ) return;
    printf("\nFinished: "); m_taskexec->stdOut( false);

    readProcess();

    if(/*( exitStatus != QProcess::NormalExit ) || */( m_stop_time != 0 ))
    {
        printf("Task terminated/killed.\n");
        if( m_update_status != af::TaskExec::UPFinishedParserError )
            m_update_status = af::TaskExec::UPFinishedCrash;
    }
    else
    {
        if( exitCode != 0)
        {
            m_update_status = af::TaskExec::UPFinishedError;
            printf("Error: exitcode = %d.\n", exitCode);
        }
        else if( m_parser->isBadResult())
        {
            m_update_status = af::TaskExec::UPFinishedParserBadResult;
            printf("Error: Bad result from m_parser (exitcode = %d).\n", exitCode);
        }
        else
        {
            m_update_status = af::TaskExec::UPFinishedSuccess;
            printf("Success: exitcode = %d.\n", exitCode);
        }
    }
    sendTaskSate();
}

void TaskProcess::stop( bool noStatusUpdate)
{
    if( noStatusUpdate ) m_update_status = 0;

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

void TaskProcess::getOutput( af::Msg & msg) const
{
    if( m_parser != NULL)
    {
        int size;
        char *data = m_parser->getData( &size);
        if( size > 0)
        {
            msg.setData( size, data);
        }
        else
        {
            msg.setString("Render: Silence...");
        }
    }
    else
    {
        msg.setString("Render: Parser is NULL.");
    }
}
