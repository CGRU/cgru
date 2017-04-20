#include "taskprocess.h"

#include <fcntl.h>
#include <sys/types.h>

#ifdef WINNT
#include <windows.h>
#include <Winsock2.h>
#define fclose CloseHandle
#define WEXITSTATUS
#else
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>
extern void (*fp_setupChildProcess)( void);
#endif

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mctaskup.h"

#include "renderhost.h"
#include "parserhost.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

#ifndef WINNT
// Setup task process for UNIX-like OSes:
// This function is called by child process just after fork() and before exec()
void setupChildProcess( void)
{
//printf("This is child process!\n");
#ifdef MACOSX
	if( setpgrp() == -1 ) AFERRPE("setpgrp")
#endif
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
#endif

long long TaskProcess::ms_counter = 0;

TaskProcess::TaskProcess( af::TaskExec * i_taskExec, RenderHost * i_render):
	m_taskexec( i_taskExec),
	m_environ( NULL),
	m_render( i_render),
	m_parser( NULL),
	m_update_status( af::TaskExec::UPPercent),
	m_stop_time( 0),
	m_pid(0),
	m_commands_launched(0),
	m_doing_post( false),
	m_closed( false),
	m_zombie( false),
	m_cycle(0),
	m_dead_cycle(0)
{
	m_store_dir = af::Environment::getStoreFolder() + AFGENERAL::PATH_SEPARATOR + "tasks" + AFGENERAL::PATH_SEPARATOR;
	m_store_dir += af::itos( m_taskexec->getJobId());
	m_store_dir += '.' + af::itos( m_taskexec->getBlockNum());
	m_store_dir += '.' + af::itos( m_taskexec->getTaskNum());
	m_store_dir += '_' + af::itos( ++ms_counter);

	if( af::pathIsFolder( m_store_dir))
		af::removeDir( m_store_dir);
	af::pathMakePath( m_store_dir);

	m_service = new af::Service( m_taskexec, m_store_dir);
	m_parser = new ParserHost( m_service);

	m_cmd = m_service->getCommand();
	AF_DEBUG << m_cmd.c_str();
	if( m_cmd.size() == 0 )
	{
		m_update_status = af::TaskExec::UPSkip;
		printf("Skipping: ");
		m_taskexec->v_stdOut( af::Environment::isVerboseMode());
		sendTaskSate();
		return;
	}
	m_taskexec->setCommand( m_cmd);


	// Process task working directory:
	m_wdir = m_service->getWDir();
	if( m_wdir.size())
	{
#ifdef WINNT
		if( m_wdir.find("\\\\") == 0)
		{
			AF_WARN << "Working directory starts with '\\':";
			printf("%s\n", m_wdir.c_str());
			AF_WARN << "UNC path can't be current";
			m_wdir.clear();
		}
#endif
		if( false == af::pathIsFolder( m_wdir))
		{
			AF_WARN << "Working directory does not exist:";
			printf("%s\n", m_wdir.c_str());
			m_wdir.clear();
		}
	}
	if( m_wdir.empty())
		m_wdir = af::Environment::getStoreFolder();
	m_taskexec->setWDir( m_wdir);

	// Process environment:
	if( m_taskexec->hasEnv())
		m_environ = af::processEnviron( m_taskexec->getEnv());

	if( af::Environment::isVerboseMode()) printf("%s\n", m_cmd.c_str());

	launchCommand();

	if( m_pid == 0 )
	{
		m_update_status = af::TaskExec::UPFailedToStart;
		sendTaskSate();
		return;
	}
}

void TaskProcess::launchCommand()
{
//printf("TaskProcess::launchCommand: command:\n%s\n", m_cmd.c_str());

	// Close handles remained from previous launch:
	closeHandles();

	m_commands_launched++;

	#ifdef WINNT
	DWORD priority = NORMAL_PRIORITY_CLASS;
	int nice = af::Environment::getRenderNice();
	if( nice >   0 ) priority = BELOW_NORMAL_PRIORITY_CLASS;
	if( nice >  10 ) priority = IDLE_PRIORITY_CLASS;
	if( nice <   0 ) priority = ABOVE_NORMAL_PRIORITY_CLASS;
	if( nice < -10 ) priority = HIGH_PRIORITY_CLASS;

	// For MSWIN we need to CREATE_SUSPENDED to attach process to a job before it can spawn any child:
	if( m_render->noOutputRedirection())
	{
		// Test a command w/o output redirection:
	    if( af::launchProgram( &m_pinfo, m_cmd, m_wdir, m_environ, 0, 0, 0,
			CREATE_SUSPENDED | priority ))
			m_pid = m_pinfo.dwProcessId;
	}
	else
	{
	    if( af::launchProgram( &m_pinfo, m_cmd, m_wdir, m_environ, &m_io_input, &m_io_output, &m_io_outerr,
			CREATE_SUSPENDED | priority ))
			m_pid = m_pinfo.dwProcessId;
	}
	#else
	// For UNIX we can ask child prcocess to call a function to setup after fork()
	fp_setupChildProcess = setupChildProcess;
	if( m_render->noOutputRedirection())
		m_pid = af::launchProgram( m_cmd, m_wdir, m_environ, 0, 0, 0);
	else
		m_pid = af::launchProgram( m_cmd, m_wdir, m_environ, &m_io_input, &m_io_output, &m_io_outerr);
	#endif

	if( m_pid <= 0 )
	{
		AFERROR("Failed to start a process")
		m_pid = 0;
		return;
	}

	#ifdef WINNT
	// On windows we attach process to a job to close all spawned childs:
	m_hjob = CreateJobObject( NULL, NULL);
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
	jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	if( SetInformationJobObject( m_hjob, JobObjectExtendedLimitInformation, &jeli, sizeof( jeli ) ) == 0)
		AFERROR("SetInformationJobObject failed.\n");
	if( AssignProcessToJobObject( m_hjob, m_pinfo.hProcess) == false)
		AFERRAR("TaskProcess: AssignProcessToJobObject failed with code = %d.", GetLastError())
	// Ppecess is CREATE_SUSPENDED so we need to resume it:
	if( ResumeThread( m_pinfo.hThread) == -1)
		AFERRAR("TaskProcess: ResumeThread failed with code = %d.", GetLastError())

	#else
	// On UNIX we set buffers and non-blocking:
	if( false == m_render->noOutputRedirection())
	{
		setbuf( m_io_output, m_filebuffer_out);
		setbuf( m_io_outerr, m_filebuffer_err);
		setNonblocking( fileno( m_io_input));
		setNonblocking( fileno( m_io_output));
		setNonblocking( fileno( m_io_outerr));
	}
	#endif


	// Just output a small log:
	std::string log = "Started";
	log += " PID=" + af::itos(m_pid);

	if( false == m_doing_post )
		log += " " + m_taskexec->v_generateInfoString( af::Environment::isVerboseMode());
	else
		log += "POST: " + m_cmd;

	int tasks = m_render->getTasksCount();
	int running = m_render->getRunningTasksCount();

	if( tasks > 0 )
	{
		log += " (";
		log += "Running tasks:" + af::itos(running);
		if( running != tasks )
			log += " / " + af::itos(tasks) + " Total";
		log += ")";
	}

	AF_LOG << log;
}

TaskProcess::~TaskProcess()
{
	AF_DEBUG << m_taskexec << " " << this;

	m_update_status = 0;

	killProcess();
	closeHandles();

	if( m_environ )
	{
		#ifndef WINNT
		for( char ** e = m_environ; *e != 0; e++)
			delete [] *e;
		#endif
		delete [] m_environ;
	}

	delete m_taskexec;
	delete m_service;
	delete m_parser;

	af::removeDir( m_store_dir);
}

void TaskProcess::closeHandles()
{
	if( m_commands_launched < 1 )
		return;

	if( false == m_render->noOutputRedirection())
	{
		fclose( m_io_input);
		fclose( m_io_output);
		fclose( m_io_outerr);
	}
}

void TaskProcess::refresh()
{
	if( m_zombie ) return;

	m_cycle++;

	AF_DEBUG << this;

	// If task was asked to stop
    if( m_stop_time )
	{
		// If it is not running any more
		if(( m_pid == 0 ) && m_closed )
		{
			// Set zombie to let render to delete this class instance
			m_zombie = true;
		}
		else if( time( NULL) - m_stop_time > AFRENDER::TERMINATEWAITKILL )
		{
			// Task was asket to stop but did not stopped for more than AFRENDER::TERMINATEWAITKILL seconds
			AF_WARN << "Task stopping time > " << AFRENDER::TERMINATEWAITKILL << " seconds.";
			// Kill process in this case
			killProcess();
		}
	}

	// Task is finished
	if( m_pid == 0 )
	{
		sendTaskSate();

		m_dead_cycle++;

		return;
	}

	int status;
	pid_t pid = 0;
#ifdef WINNT
	DWORD result = WaitForSingleObject( m_pinfo.hProcess, 0);
	if ( result == WAIT_OBJECT_0)
	{
		GetExitCodeProcess( m_pinfo.hProcess, &result);
		status = result;
		pid = m_pid;
	}
	else if ( result == WAIT_FAILED )
	{
		pid = -1;
	}
#else

	pid = waitpid( m_pid, &status, WNOHANG);
#endif

	if( pid == 0 )
	{
		// Task is not finished
		readProcess("RUN");
	}
	else if( pid == m_pid )
	{
		// Task is finished
		processFinished( status);
	}
	else if( pid == -1 )
	{
		AFERRPE("TaskProcess::refresh(): waitpid: ")
	}

	sendTaskSate();
}

void TaskProcess::close()
{
// Server asked render to close a task
// So it is not needed for server any more
	if( m_closed )
	{
		AF_WARN << "Closed task was asked to close again: " << this;
		return;
	}

	AF_DEBUG << this;

	m_closed = true;

	// Zero value means that message for server is not needed
   	m_update_status = 0;

	if( m_stop_time )
	{
		// This task was asked to stop
		// It can be parser case, error or success, no matter here
		// Task should be set to zombie only after its process finish
		return;
	}

	if( m_pid )
	{
		// Unusual case.
		// This task is running, but was asked to close by server.
		// May be afserver after restart lost the job.
		stop();
		return;
	}

	// Set zombie to let render to delete this class instance
	m_zombie = true;
}

void TaskProcess::readProcess( const std::string & i_mode)
{
	AF_DEBUG << this;

	if( m_render->noOutputRedirection()) return;

	std::string output;

	int readsize = readPipe( m_io_output);
	if( readsize > 0 )
		output = std::string( m_readbuffer, readsize);

	readsize = readPipe( m_io_outerr);
	if( readsize > 0 )
		output += std::string( m_readbuffer, readsize);

	m_parser->read( i_mode, output);

	if( output.size() && m_taskexec->isListening())
	{
		m_listened += output;
	}

	if( m_parser->hasWarning() && ( m_update_status != af::TaskExec::UPWarning               ) &&
	                              ( m_update_status != af::TaskExec::UPFinishedParserError   ) &&
	                              ( m_update_status != af::TaskExec::UPFinishedParserSuccess ))
	{
		AF_LOG << "Parser notification.";
		m_update_status = af::TaskExec::UPWarning;
	}

	// if task is not in "stopping" state
	if( m_stop_time == 0 )
	{
		// ckeck parser reasons to force to stop a task
	    if( m_parser->hasError())
	    {
	        AF_LOG << "Parser bad result. Stopping task.";
	        m_update_status = af::TaskExec::UPFinishedParserError;
	        stop();
	    }
	    if( m_parser->isFinishedSuccess())
	    {
	        AF_LOG << "Parser finished success. Stopping task.";
	        m_update_status = af::TaskExec::UPFinishedParserSuccess;
	        stop();
	    }
	}
}

void TaskProcess::sendTaskSate()
{
	if( m_update_status == 0 )
	{
		// Zero value means that message for server is not needed
		return;
	}

	AF_DEBUG << this;

	bool   toRecieve = false;
	char * stdout_data = NULL;
	int    stdout_size = 0;
	std::string log;

	if(( m_update_status != af::TaskExec::UPPercent ) &&
		( m_update_status != af::TaskExec::UPWarning ))
	{
		toRecieve = true;
		stdout_data = m_parser->getData( &stdout_size);
		log = m_service->getLog();
	}

	int percent          = m_parser->getPercent();
	int frame            = m_parser->getFrame();
	int percentframe     = m_parser->getPercentFrame();
	std::string activity = m_parser->getActivity();
	std::string report   = m_parser->getReport();


	if( m_render->notConnected() || ( m_dead_cycle ))
	{
		if( m_render->isConnected())
			printf("Dead Cycle #%d: ", m_dead_cycle);

		switch( m_update_status )
		{
			case af::TaskExec::UPFinishedSuccess:
				printf("DON");
				break;
			case af::TaskExec::UPFinishedParserBadResult:
			case af::TaskExec::UPFinishedParserError:
			case af::TaskExec::UPFinishedError:
				printf("ERR");
				break;
			default:
				printf("%d%%", percent);
		}

		printf(" : ");
		m_taskexec->v_stdOut();
	}


	af::MCTaskUp * taskup = new af::MCTaskUp(
		m_render->getId(),

		m_taskexec->getJobId(),
		m_taskexec->getBlockNum(),
		m_taskexec->getTaskNum(),
		m_taskexec->getNumber(),

		m_update_status,
		percent,
		frame,
		percentframe,

		log,
		activity,
		report,

		m_listened,

		stdout_size,
		stdout_data);

	collectFiles( *taskup);
	taskup->setParsedFiles( m_service->getParsedFiles());

	m_listened.clear();

	m_render->addTaskUp( taskup);
}

void TaskProcess::processFinished( int i_exitCode)
{
	AF_LOG << "Finished PID=" << m_pid << ": Exit Code=" << i_exitCode
		<< " Status=" << WEXITSTATUS( i_exitCode) << (m_stop_time ? " (stopped)":"");

	// Zero m_pid means that task is not running any more
	m_pid = 0;

#ifdef WINNT
	if( m_stop_time != 0 )
		AF_LOG << "Task terminated/killed.";
#else
	if(( m_stop_time != 0 ) || WIFSIGNALED( i_exitCode))
		AF_LOG << "Task terminated/killed by signal: '" << strsignal( WTERMSIG( i_exitCode)) << "'";
#endif

	// If server update not needed
	if( m_update_status == 0 )
	{
		// We do not need to read process output
		return;
	}

	// Read process last output
	readProcess( af::itos( i_exitCode) + ':' + af::itos( m_stop_time));

#ifdef WINNT
	bool success = m_service->checkExitStatus( i_exitCode);
#else
	bool success = false;
	if( WIFEXITED( i_exitCode))
		success = m_service->checkExitStatus( WEXITSTATUS( i_exitCode));
#endif

	if(( success != true ) || ( m_stop_time != 0 ))
	{
		if( m_doing_post )
			m_update_status = af::TaskExec::UPFinishedFailedPost;
		else
			m_update_status = af::TaskExec::UPFinishedError;
#ifdef WINNT
		if( m_stop_time != 0 )
#else
		if(( m_stop_time != 0 ) || WIFSIGNALED( i_exitCode))
#endif
		{
			if(( m_update_status != af::TaskExec::UPFinishedParserError   ) &&
			   ( m_update_status != af::TaskExec::UPFinishedParserSuccess ))
			     m_update_status  = af::TaskExec::UPFinishedKilled;
		}
	}
	else if( m_parser->isBadResult())
	{
		m_update_status = af::TaskExec::UPFinishedParserBadResult;
		AF_LOG << "Bad result from parser.";
	}
	else if( m_taskexec->hasFileSizeCheck() &&
		( false == m_service->checkRenderedFiles()))
	{
		m_update_status = af::TaskExec::UPBadRenderedFiles;
	}
	else
	{
		if( false == m_doing_post )
		{
			m_post_cmds = m_service->doPost();
			m_doing_post = true;
		}

		if( m_doing_post && m_post_cmds.size())
		{
			m_cmd = m_post_cmds.front();
			m_post_cmds.erase( m_post_cmds.begin());
			launchCommand();
		}
		else
			m_update_status = af::TaskExec::UPFinishedSuccess;
	}
}

void TaskProcess::stop()
{
	if( m_stop_time )
	{
		AF_WARN << "Stopped task was asked to stop again: " << this;
		return;
	}

	AF_DEBUG << this;

	// Return if task is not running
	if( m_pid == 0 )
	{
		return;
	}

	// Store the time when task was asked to be stopped (was asked first time)
	m_stop_time = time(NULL);

	// Trying to terminate() first, and only if no response after some time, then perform kill()
#ifdef UNIX
	killpg( getpgid( m_pid), SIGTERM);
#else
	CloseHandle( m_hjob );
#endif
}

void TaskProcess::killProcess()
{
	if( m_pid == 0 ) return;

	AF_DEBUG << this;

	AF_WARN << "KILLING NOT TERMINATED TASK.";
#ifdef UNIX
	killpg( getpgid( m_pid), SIGKILL);
#else
	CloseHandle( m_hjob );
#endif
}

const std::string TaskProcess::getOutput() const
{
	int size;
	char *data = m_parser->getData( &size);

	if( size < 1 )
		return "Render task process: Silence...";

	return std::string( data, size);
}

#ifdef WINNT
int TaskProcess::readPipe( HANDLE i_handle )
{
	// Get how much data is available:
	DWORD bytesAvail;
	if ( false == PeekNamedPipe( i_handle, NULL, 0, NULL, &bytesAvail, NULL))
	{
		DWORD lastError = GetLastError();
		if( lastError != ERROR_BROKEN_PIPE )
			AFERRAR("TaskProcess::readPipe: PeekNamedPipe() failure with code = %d.", GetLastError())
		return 0;
	}

	if ( bytesAvail <= 0 )
		return 0;

	// Read data:
	DWORD readsize = 0;
	if( false == ReadFile( i_handle, m_readbuffer, bytesAvail, &readsize, NULL))
	{
		AFERRAR("TaskProcess::readPipe: ReadFile() failure with code = %d.", GetLastError())
		return 0;
	}

	return readsize;
}
#else
int TaskProcess::readPipe( FILE * i_file )
{
	int readsize = fread( m_readbuffer, 1, m_readbuffer_size, i_file );
	if( readsize <= 0 )
	{
		if( errno == EAGAIN )
		{
			readsize = fread( m_readbuffer, 1, m_readbuffer_size, i_file );
			if( readsize <= 0 )
			    return 0;
		}
		else
			return 0;
	}
	return readsize;
}
#endif

void TaskProcess::collectFiles( af::MCTaskUp & i_task_up)
{
	std::vector<std::string> list = af::getFilesList( m_store_dir);
	for( int i = 0; i < list.size(); i++)
	{
		bool already_collected = false;
		for( int j = 0; j < m_collected_files.size(); j++)
		{
			if( m_collected_files[j] == list[i] )
			{
				//printf("File '%s' already collected\n", list[i].c_str());
				already_collected = true;
				break;
			}
		}
		if( already_collected ) continue;
		m_collected_files.push_back( list[i]);

		std::string path = m_store_dir + AFGENERAL::PATH_SEPARATOR + list[i];
		#ifdef WINNT
		path = af::strReplace( path, '/','\\');
		#endif
		int size;
		char * data = af::fileRead( path, &size, 100000);
		if( data == NULL ) continue;
		if( size >= 100000 )
		{
			delete data;
			continue;
		}
		i_task_up.addFile( list[i], data, size);
	}
}

const std::string TaskProcess::generateInfoString( bool i_full) const
{
	std::ostringstream str;

	generateInfoStream( str);

	return str.str();
}

void TaskProcess::generateInfoStream( std::ostringstream & o_str, bool i_full) const
{
//	AF_DEBUG << "PID=" << m_pid << ", zombie=" << (m_zombie ? "TRUE":"FALSE") << ", stop_time=" << m_stop_time;
//		<< "Update Satus = " << m_update_status;
	o_str << "PID=" << m_pid;

	if( m_stop_time ) o_str << " STOPPING";

	if( m_closed ) o_str << " CLOSED";

	if( m_zombie ) o_str << " ZOMBIE";

	o_str << " UP:" << int(m_update_status);
}

