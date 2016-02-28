#include "../libafanasy/common/dlThread.h"

#ifndef WINNT
#include <sys/wait.h>
#endif

#include "../libafanasy/environment.h"
#include "../libafanasy/host.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/render.h"

#include "res.h"
#include "renderhost.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

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
void threadAcceptClient( void * i_arg );
void msgCase( af::Msg * msg);
void launchAndExit( af::Msg * i_msg, bool i_exit);

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
	if( af::pathMakePath( ENV.getTempDir(), af::VerboseOn ) == false) return 1;

	RenderHost * render = new RenderHost();

	DlThread ServerAccept;
	ServerAccept.Start( &threadAcceptClient, NULL);

	uint64_t cycle = 0;
	while( AFRunning)
	{
		if( false == RenderHost::isListening() )
		{
			// Wait accept thread to start to listen a port.
			af::sleep_msec( 100);
			continue;
		}

		// Collect all available incomming messages:
		std::list<af::Msg*> in_msgs;
		while( af::Msg * msg = RenderHost::acceptTry() )
			in_msgs.push_back( msg);

		// Lock render:
		RenderHost::lockMutex();
		// React on all incoming messages:
		for( std::list<af::Msg*>::iterator it = in_msgs.begin(); it != in_msgs.end(); it++)
			msgCase( *it);
		// Let tasks to do their work:
		RenderHost::refreshTasks();
		// Unlock render:
		RenderHost::unLockMutex();

		// Update render resources:
		if( cycle % af::Environment::getRenderUpdateSec() == 0)
			RenderHost::update();

		cycle++;

		if( AFRunning )
			af::sleep_sec(1);
	}

	delete render;

	Py_Finalize();

	printf("Exiting render.\n");

	return 0;
}

void msgCase( af::Msg * msg)
{
	if( false == AFRunning )
		return;

	if( msg == NULL)
	{
		return;
	}
#ifdef AFOUTPUT
printf("msgCase: "); msg->stdOut();
#endif

	// Check not sended messages first, they were pushed back in accept quere:
	if( msg->wasSendFailed())
	{
		if( msg->getAddress().equal( af::Environment::getServerAddress()))
		{
			// Message was failed to send to server
			RenderHost::connectionLost();
		}
		else if( msg->type() == af::Msg::TTaskOutput )
		{
			RenderHost::listenFailed( msg->getAddress());
		}
		delete msg;
		return;
	}

	switch( msg->type())
	{
	case af::Msg::TRenderId:
	{
		int new_id = msg->int32();
		// Server sends back -1 id if a render with the same hostname already exists:
		if( new_id == -1)
		{
			AFERRAR("Render with this hostname '%s' already registered.", af::Environment::getHostName().c_str())
			AFRunning = false;
		}
		// Render was trying to register (its id==0) and server has send id>0
		// This is the situation when client was sucessfully registered
		else if((new_id > 0) && (RenderHost::getId() == 0))
		{
			RenderHost::setRegistered( new_id);
		}
		// Server sends back zero id on any error
		else if ( new_id == 0 )
		{
			printf("Zero ID recieved, no such online render, re-connecting...\n");
			RenderHost::connectionLost( true);
		}
		// Bad case, should not ever happen, try to re-register.
		else if ( RenderHost::getId() != new_id )
		{
			AFERRAR("IDs mistatch: this %d != %d new, re-connecting...", RenderHost::getId(), new_id);
			RenderHost::connectionLost( true);
		}
		// Id, that returns from server is equals to stored on client.
		// This is a normal case.
		else
		{
			RenderHost::connectionEstablished();
		}
		break;
	}
	case af::Msg::TVersionMismatch:
	case af::Msg::TClientExitRequest:
	{
		printf("Render exit request received.\n");
		AFRunning = false;
		break;
	}
	case af::Msg::TTask:
	{
		RenderHost::runTask( msg);
		break;
	}
/*	case af::Msg::TClientRestartRequest:
	{
		printf("Restart client request, executing command:\n%s\n", af::Environment::getRenderExec().c_str());
		af::launchProgram(af::Environment::getRenderExec());
		AFRunning = false;
		break;
	}*/
//   case af::Msg::TClientStartRequest:
//   {
//	  printf("Start client request, executing command:\n%s\n", af::Environment::getRenderExec().c_str());
//	  af::launchProgram( af::Environment::getRenderExec());
//	  break;
//   }
	case af::Msg::TClientRebootRequest:
	{
		AFRunning = false;
		printf("Reboot request, executing command:\n%s\n", af::Environment::getRenderCmdReboot().c_str());
		af::launchProgram( af::Environment::getRenderCmdReboot());
		break;
	}
	case af::Msg::TClientShutdownRequest:
	{
		AFRunning = false;
		printf("Shutdown request, executing command:\n%s\n", af::Environment::getRenderCmdShutdown().c_str());
		af::launchProgram( af::Environment::getRenderCmdShutdown());
		break;
	}
	case af::Msg::TRenderWOLSleepRequest:
	{
		af::MCGeneral mg( msg);
		RenderHost::wolSleep( mg.getString());
		break;
	}
	case af::Msg::TRenderStopTask:
	{
		af::MCTaskPos taskpos( msg);
		RenderHost::stopTask( taskpos);
		break;
	}
	case af::Msg::TRenderCloseTask:
	{
		af::MCTaskPos taskpos( msg);
		RenderHost::closeTask( taskpos);
		break;
	}
	case af::Msg::TTaskListenOutput:
	{
		af::MCListenAddress mcaddr( msg);
		RenderHost::listenTasks( mcaddr);
		break;
	}
	case af::Msg::TRenderLaunch:
	{
		launchAndExit( msg, false);
		break;
	}
	case af::Msg::TRenderLaunchAndExit:
	{
		launchAndExit( msg, true);
		break;
	}
	default:
	{
		AFERROR("Unknown message recieved:")
		msg->v_stdOut();
		break;
	}
	}

	delete msg;
}

void launchAndExit( af::Msg * i_msg, bool i_exit)
{
	af::MCGeneral mcg( i_msg);

	std::string cmd = mcg.getString();

	if( i_exit )
	{
		printf("%s\n%s\n","Launching command and exiting:", cmd.c_str());
		AFRunning = false;
	}
	else
	{
		printf("%s\n%s\n","Launching command:", cmd.c_str());
	}

	af::launchProgram( cmd);
}

