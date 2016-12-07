//#include "../libafanasy/common/dlThread.h"

#ifndef WINNT
#include <sys/wait.h>
#endif

#include "../libafanasy/environment.h"
#include "../libafanasy/host.h"
#include "../libafanasy/render.h"
#include "../libafanasy/renderevents.h"

#include "res.h"
#include "renderhost.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

extern bool AFRunning;

//####################### interrupt signal handler ####################################
#include <signal.h>
void sig_pipe(int signum)
{
	AFERROR("AFRender SIGPIPE");
}
void sig_int(int signum)
{
	if( AFRunning )
		fprintf( stderr,"\nAFRender: Interrupt signal catched.\n");
	AFRunning = false;
}
//#####################################################################################

// Functions:
//void threadAcceptClient( void * i_arg );
void msgCase( af::Msg * i_msg, RenderHost & i_render);
void processEvents( const af::RenderEvents & i_re, RenderHost & i_render);
void launchAndExit( const std::string & i_str, bool i_exit);

int main(int argc, char *argv[])
{
	Py_InitializeEx(0);

   // Set signals handlers:
#ifdef WINNT
	signal( SIGINT,  sig_int);
	signal( SIGTERM, sig_int);
	signal( SIGSEGV, sig_int);
#else
	struct sigaction actint;
	bzero( &actint, sizeof(actint));
	actint.sa_handler = sig_int;
	sigaction( SIGINT,  &actint, NULL);
	sigaction( SIGTERM, &actint, NULL);
	sigaction( SIGSEGV, &actint, NULL);
	// SIGPIPE signal catch:
	struct sigaction actpipe;
	bzero( &actpipe, sizeof(actpipe));
	actpipe.sa_handler = sig_pipe;
	sigaction( SIGPIPE, &actpipe, NULL);
#endif

	// Initialize environment and try to append python path:
	af::Environment ENV( af::Environment::AppendPythonPath | af::Environment::SolveServerName, argc, argv);
	if( !ENV.isValid())
	{
		AFERROR("main: Environment initialization failed.\n");
		exit(1);
	}

	// Fill command arguments:
	ENV.addUsage("-nimby", "Set initial state to 'nimby'.");
	ENV.addUsage("-NIMBY", "Set initial state to 'NIMBY'.");
	ENV.addUsage( std::string("-cmd") + " [command]", "Run command only, do not connect to server.");
	ENV.addUsage("-res", "Check host resources only and quit.");
	ENV.addUsage("-nor", "No output redirection.");
	// Help mode, usage is alredy printed, exiting:
	if( ENV.isHelpMode() )
		return 0;

	// Check resources and exit:
	if( ENV.hasArgument("-res"))
	{
		af::Host host;
		af::HostRes hostres;
		GetResources( host, hostres, true);
		af::sleep_msec(100);
		GetResources( host, hostres);
		printf("\n");
		host.v_stdOut( true);
		hostres.v_stdOut( true);
		Py_Finalize();
		return 0;
	}

	// Run command and exit
	if( ENV.hasArgument("-cmd"))
	{
		std::string command;
		ENV.getArgument("-cmd", command);
		printf("Test command mode:\n%s\n", command.c_str());

		pid_t m_pid;
		int status;
		pid_t pid = 0;
		#ifdef WINNT
		PROCESS_INFORMATION m_pinfo;
    	if( af::launchProgram( &m_pinfo, command, "", 0, 0, 0))
			m_pid = m_pinfo.dwProcessId;
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
		m_pid = af::launchProgram( command, "", 0, 0, 0);
		pid = waitpid( m_pid, &status, 0);
		#endif

		Py_Finalize();
		return 0;
	}

	// Create temp directory, if it does not exist:
	if( af::pathMakePath( ENV.getStoreFolder(), af::VerboseOn ) == false) return 1;

	RenderHost * render = RenderHost::getInstance();

	uint64_t cycle = 0;
	while( AFRunning)
	{
		// Update machine resources:
		if( cycle % af::Environment::getRenderUpResourcesPeriod() == 0)
			render->getResources();

		// Let tasks to do their work:
		render->refreshTasks();

		// Update server (send info and receive an answer):
		af::Msg * answer = render->updateServer();

		// React on a server answer:
		msgCase( answer, *render);

		// Close windows on windows:
		#ifdef WINNT
		render->windowsMustDie();
		#endif

		// Increment cycle:
		cycle++;
		#ifdef AFOUTPUT
		printf("=============================================================\n\n");
		#endif

		// Sleep till the next heartbeat:
		if( AFRunning )
			af::sleep_sec( af::Environment::getRenderHeartbeatSec());
	}

	delete render;

	Py_Finalize();

    AF_LOG << "Exiting render.";

	return 0;
}

void msgCase( af::Msg * i_msg, RenderHost & i_render)
{
	if( i_msg == NULL)
	{
		return;
	}

	if( false == AFRunning )
		return;

#ifdef AFOUTPUT
AF_LOG << " >>> " << i_msg;
#endif

	switch( i_msg->type())
	{
	case af::Msg::TRenderId:
	{
		int new_id = i_msg->int32();
		// Server sends back -1 id if a render with the same hostname already exists:
		if( new_id == -1)
		{
			AF_ERR << "Render with this hostname '" << af::Environment::getHostName() << "' already registered.";
			AFRunning = false;
		}
		// Render was trying to register (its id==0) and server has send id>0
		// This is the situation when client was sucessfully registered
//		else if((new_id > 0) && (i_render.getId() == 0))
		else if((new_id > 0) && i_render.notConnected())
		{
			i_render.setRegistered( new_id);
		}
		// Server sends back zero id on any error
		else if ( new_id == 0 )
		{
			AF_ERR << "Zero ID recieved, no such online render, re-connecting...";
			i_render.connectionLost( true);
		}
		// Bad case, should not ever happen, try to re-register.
		else if ( i_render.getId() != new_id )
		{
			AF_ERR << "IDs mistatch: this " << i_render.getId() << " != " << new_id << " new, re-connecting...";
			i_render.connectionLost( true);
		}
		// Id, that returns from server is equals to stored on client.
		// This is a normal case.
		else
		{
			i_render.connectionEstablished();
		}
		break;
	}
	case af::Msg::TVersionMismatch:
	{
		AF_LOG << "Render exit request received.";
		AFRunning = false;
		break;
	}
	case af::Msg::TRenderEvents:
	{
		af::RenderEvents me( i_msg);
		processEvents( me, i_render);
		break;
	}
	default:
	{
        AF_ERR << "Unknown message recieved: " << *i_msg;
		break;
	}
	}

	delete i_msg;
}

void processEvents( const af::RenderEvents & i_re, RenderHost & i_render)
{
#ifdef AFOUTPUT
AF_LOG << i_re;
#endif

	// Tasks to execute:
	for( int i = 0; i < i_re.m_tasks.size(); i++)
		i_render.runTask( i_re.m_tasks[i]);


	// Tasks to close:
	for( int i = 0; i < i_re.m_closes.size(); i++)
		i_render.closeTask( i_re.m_closes[i]);


	// Tasks to stop:
	for( int i = 0; i < i_re.m_stops.size(); i++)
		i_render.stopTask( i_re.m_stops[i]);


	// Tasks to outputs:
	for( int i = 0; i < i_re.m_outputs.size(); i++)
		i_render.upTaskOutput( i_re.m_outputs[i]);

	// Listens add:
	for( int i = 0; i < i_re.m_listens_add.size(); i++)
		i_render.listenTask( i_re.m_listens_add[i], true);

	// Listens remove:
	for( int i = 0; i < i_re.m_listens_rem.size(); i++)
		i_render.listenTask( i_re.m_listens_rem[i], false);

	// Instructions:
	if( i_re.m_instruction.size())
	{
		if( i_re.m_instruction == "exit")
		{
			AF_LOG << "Render exit request received.";
			AFRunning = false;
		}
		else if( i_re.m_instruction == "sleep")
		{
			AF_LOG << "Render sleep request received.";
			i_render.wolSleep( i_re.m_command);
		}
		else if( i_re.m_instruction == "launch")
		{
			launchAndExit( i_re.m_command, false);
		}
		else if( i_re.m_instruction == "launch_exit")
		{
			launchAndExit( i_re.m_command, true);
		}
		else if( i_re.m_instruction == "reboot")
		{
			AFRunning = false;
			AF_LOG << "Reboot request, executing command:\n"
			       << af::Environment::getRenderCmdReboot();
			af::launchProgram( af::Environment::getRenderCmdReboot());
		}
		else if( i_re.m_instruction == "shutdown")
		{
			AFRunning = false;
			AF_LOG << "Shutdown request, executing command:\n"
			       << af::Environment::getRenderCmdShutdown();
			af::launchProgram( af::Environment::getRenderCmdShutdown());
		}
	}
}

void launchAndExit( const std::string & i_cmd, bool i_exit)
{
	if( i_exit )
	{
		AF_LOG << "Launching command and exiting:\n"
		       << i_cmd;
		AFRunning = false;
	}
	else
	{
		AF_LOG << "Launching command:\n"
		       << i_cmd;
	}

	af::launchProgram( i_cmd);
}

