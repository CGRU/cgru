//#include "../libafanasy/common/dlThread.h"

#ifndef WINNT
#include <sys/wait.h>
#endif

#include "../libafanasy/environment.h"
#include "../libafanasy/host.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/render.h"
#include "../libafanasy/renderevents.h"

#include "res.h"
#include "renderhost.h"

#define AFOUTPUT
//#undef AFOUTPUT
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
//void threadAcceptClient( void * i_arg );
void msgCase( af::Msg * msg);
void processEvents( const af::RenderEvents & i_me);
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
	if( af::pathMakePath( ENV.getTempDir(), af::VerboseOn ) == false) return 1;

	RenderHost * render = new RenderHost();

//	DlThread ServerAccept;
//	ServerAccept.Start( &threadAcceptClient, NULL);

	uint64_t cycle = 0;
	while( AFRunning)
	{
		// Collect all available incomming messages:
		std::list<af::Msg*> in_msgs;
		while( af::Msg * msg = RenderHost::acceptTry() )
			in_msgs.push_back( msg);

		// Lock render:
//		RenderHost::lockMutex(); // Why we need mutex here?

		// React on all incoming messages:
		for( std::list<af::Msg*>::iterator it = in_msgs.begin(); it != in_msgs.end(); it++)
			msgCase( *it);
		// Let tasks to do their work:
		RenderHost::refreshTasks();

		// Update render:
		RenderHost::update( cycle);

		// Unlock render:
//		RenderHost::unLockMutex();

		cycle++;

		#ifdef AFOUTPUT
		printf("=============================================================\n\n");
		#endif

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
printf(" >>> "); msg->v_stdOut();
#endif

	// Check not sended messages first, they were pushed back in accept quere:
	if( msg->wasSendFailed())
	{
		if( msg->getAddress().equal( af::Environment::getServerAddress()))
		{
			// Message was failed to send to server
			RenderHost::connectionLost();
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
	{
		printf("Render exit request received.\n");
		AFRunning = false;
		break;
	}
	case af::Msg::TRenderEvents:
	{
		af::RenderEvents me( msg);
		processEvents( me);
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

void processEvents( const af::RenderEvents & i_me)
{
#ifdef AFOUTPUT
i_me.v_stdOut();
#endif

	// Tasks to execute:
	for( int i = 0; i < i_me.m_tasks.size(); i++)
		RenderHost::runTask( i_me.m_tasks[i]);


	// Tasks to close:
	for( int i = 0; i < i_me.m_closes.size(); i++)
		RenderHost::closeTask( i_me.m_closes[i]);


	// Tasks to stop:
	for( int i = 0; i < i_me.m_stops.size(); i++)
		RenderHost::stopTask( i_me.m_stops[i]);


	// Tasks to outputs:
	for( int i = 0; i < i_me.m_outputs.size(); i++)
		RenderHost::upTaskOutput( i_me.m_outputs[i]);

	// Listens add:
	for( int i = 0; i < i_me.m_listens_add.size(); i++)
		RenderHost::listenTask( i_me.m_listens_add[i], true);

	// Listens remove:
	for( int i = 0; i < i_me.m_listens_rem.size(); i++)
		RenderHost::listenTask( i_me.m_listens_rem[i], false);

	// Instructions:
	if( i_me.m_instruction.size())
	{
		if( i_me.m_instruction == "exit")
		{
			printf("Render exit request received.\n");
			AFRunning = false;
		}
		else if( i_me.m_instruction == "sleep")
		{
			printf("Render sleep request received.\n");
			RenderHost::wolSleep( i_me.m_command);
		}
		else if( i_me.m_instruction == "launch")
		{
			launchAndExit( i_me.m_command, false);
		}
		else if( i_me.m_instruction == "launch_exit")
		{
			launchAndExit( i_me.m_command, true);
		}
		else if( i_me.m_instruction == "reboot")
		{
			AFRunning = false;
			printf("Reboot request, executing command:\n%s\n", af::Environment::getRenderCmdReboot().c_str());
			af::launchProgram( af::Environment::getRenderCmdReboot());
		}
		else if( i_me.m_instruction == "shutdown")
		{
			AFRunning = false;
			printf("Shutdown request, executing command:\n%s\n", af::Environment::getRenderCmdShutdown().c_str());
			af::launchProgram( af::Environment::getRenderCmdShutdown());
		}
	}
}

void launchAndExit( const std::string & i_cmd, bool i_exit)
{
	if( i_exit )
	{
		printf("%s\n%s\n","Launching command and exiting:", i_cmd.c_str());
		AFRunning = false;
	}
	else
	{
		printf("%s\n%s\n","Launching command:", i_cmd.c_str());
	}

	af::launchProgram( i_cmd);
}

