/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

#ifndef WINNT
#include <signal.h>
#include <sys/wait.h>
#endif

#include "../libafanasy/environment.h"
#include "../libafanasy/hostres.h"
#include "../libafanasy/render.h"
#include "../libafanasy/renderevents.h"

#include "res.h"
#include "renderhost.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

extern bool AFRunning;

int HeartBeatSec = AFRENDER::HEARTBEAT_SEC;
int ResourcesUpdatePeriod = AFRENDER::RESOURCES_UPDATE_PERIOD;
int ZombieTime = AFRENDER::ZOMBIETIME;
int ExitNoTaskTime = -1;

// ######################### Signal handlers ############################################
#ifdef WINNT
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		case CTRL_C_EVENT: AF_LOG << "Ctrl-C event\n"; break;
		case CTRL_CLOSE_EVENT: AF_LOG << "Ctrl-Close event\n"; break;
		case CTRL_BREAK_EVENT: AF_LOG << "Ctrl-Break event\n"; break;
		case CTRL_LOGOFF_EVENT: AF_LOG << "Ctrl-Logoff event\n"; break;
		case CTRL_SHUTDOWN_EVENT: AF_LOG << "Ctrl-Shutdown event\n"; break;
		default: AF_LOG << "Ctrl-UNKNOWN event\n"; return FALSE;
	}
	AFRunning = false;
	return TRUE;
}
#else
void sig_pipe(int signum) { AF_ERR << "SIGPIPE"; }
void sig_int(int signum)
{
	if (AFRunning)
		fprintf(stderr, "\nAFRender: Interrupt signal catched.\n");
	AFRunning = false;
}
#endif
// #####################################################################################

// Functions:
// void threadAcceptClient( void * i_arg );
void msgCase(af::Msg *i_msg, RenderHost &i_render);
void processEvents(const af::RenderEvents &i_re, RenderHost &i_render);
void launchAndExit(const std::string &i_str, bool i_exit);

int main(int argc, char *argv[])
{
	Py_InitializeEx(0);

	// Set signals handlers:
#ifdef WINNT
	if (false == SetConsoleCtrlHandler(CtrlHandler, TRUE))
		AF_ERR << "SetConsoleCtrlHandler: " << af::GetLastErrorStdStr() << "\n";
#else
	struct sigaction actint;
	bzero(&actint, sizeof(actint));
	actint.sa_handler = sig_int;
	sigaction(SIGINT, &actint, NULL);
	sigaction(SIGTERM, &actint, NULL);
	// SIGPIPE signal catch:
	struct sigaction actpipe;
	bzero(&actpipe, sizeof(actpipe));
	actpipe.sa_handler = sig_pipe;
	sigaction(SIGPIPE, &actpipe, NULL);
#endif

	// Initialize environment and try to append python path:
	af::Environment ENV(af::Environment::AppendPythonPath | af::Environment::SolveServerName, argc, argv);
	if (!ENV.isValid())
	{
		AFERROR("main: Environment initialization failed.\n");
		exit(1);
	}

	// Fill command arguments:
	ENV.addUsage("-nimby", "Set initial state to 'nimby'.");
	ENV.addUsage("-NIMBY", "Set initial state to 'NIMBY'.");
	ENV.addUsage(std::string("-cmd") + " [command]", "Run command only, do not connect to server.");
	ENV.addUsage("-res", "Check host resources only and quit.");
	ENV.addUsage("-nor", "No output redirection.");
	// Help mode, usage is alredy printed, exiting:
	if (ENV.isHelpMode())
		return 0;

	// Check resources and exit:
	if (ENV.hasArgument("-res"))
	{
		af::HostRes hostres;
		GetResources(hostres, true);
		af::sleep_msec(100);
		GetResources(hostres);
		printf("\n");
		hostres.v_stdOut(true);
		Py_Finalize();
		return 0;
	}

	// Run command and exit
	if (ENV.hasArgument("-cmd"))
	{
		std::string command;
		ENV.getArgument("-cmd", command);
		printf("Test command mode:\n%s\n", command.c_str());

		pid_t m_pid;
		int status;
		pid_t pid = 0;
#ifdef WINNT
		PROCESS_INFORMATION m_pinfo;
		if (af::launchProgram(&m_pinfo, command, "", 0, 0, 0))
			m_pid = m_pinfo.dwProcessId;
		DWORD result = WaitForSingleObject(m_pinfo.hProcess, 0);
		if (result == WAIT_OBJECT_0)
		{
			GetExitCodeProcess(m_pinfo.hProcess, &result);
			status = result;
			pid = m_pid;
		}
		else if (result == WAIT_FAILED)
		{
			pid = -1;
		}
#else
		m_pid = af::launchProgram(command, "", 0, 0, 0);
		pid = waitpid(m_pid, &status, 0);
#endif

		Py_Finalize();
		return 0;
	}

	// Create temp directory, if it does not exist:
	if (af::pathMakePath(ENV.getStoreFolder(), af::VerboseOn) == false)
		return 1;

	RenderHost *render = RenderHost::getInstance();

	uint64_t cycle = 0;
	while (AFRunning)
	{
		// Update machine resources:
		if ((cycle % ResourcesUpdatePeriod) == 0)
			render->getResources();

		// Let tasks to do their work:
		render->refreshTasks();

		// Update server (send info and receive an answer):
		af::Msg *answer = render->updateServer();

		// React on a server answer:
		msgCase(answer, *render);

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
		if (AFRunning)
			af::sleep_sec(HeartBeatSec);
	}

	delete render;

	Py_Finalize();

	AF_LOG << "Exiting render.";

	return 0;
}

void msgCase(af::Msg *i_msg, RenderHost &i_render)
{
	if (i_msg == NULL)
	{
		return;
	}

	if (false == AFRunning)
		return;

#ifdef AFOUTPUT
	AF_LOG << " >>> " << i_msg;
#endif

	switch (i_msg->type())
	{
		case af::Msg::TRenderId:
		{
			// If there is no render events, server just returns render ID.
			int id = i_msg->int32();

			// Server sends back -1 id if a render with the same hostname already exists:
			if (id == -1)
			{
				AF_ERR << "Render with this hostname '" << af::Environment::getHostName()
					   << "' already registered.";
				AFRunning = false;
			}
			// Server sends back zero id on any error
			else if (id == 0)
			{
				AF_WARN << "Zero ID received, no such online render, re-connecting...";
				i_render.connectionLost();
			}
			// Bad case, should not ever happen, try to re-register.
			else if (i_render.getId() != id)
			{
				AF_ERR << "IDs mismatch: this " << i_render.getId() << " != " << id
					   << " new, re-connecting...";
				i_render.connectionLost();
			}
			// Id, that returns from server equals to stored on client.
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
			af::RenderEvents me(i_msg);
			processEvents(me, i_render);
			break;
		}
		default:
		{
			AF_ERR << "Unknown message received: " << *i_msg;
			break;
		}
	}

	delete i_msg;
}

void processEvents(const af::RenderEvents &i_re, RenderHost &i_render)
{
#ifdef AFOUTPUT
	AF_LOG << i_re;
#endif

	// Server can send some special IDs
	switch (i_re.m_id)
	{
		case af::RenderEvents::RE_Status_Reconnect:
			if (i_re.m_log.size())
				AF_WARN << "SERVER: " << i_re.m_log;
			else
				AF_WARN << "Reconnect signal received from server.";
			i_render.connectionLost();
			return;

		case af::RenderEvents::RE_Status_Exit:
			if (i_re.m_log.size())
				AF_ERR << "SERVER: " << i_re.m_log;
			else
				AF_ERR << "Exit signal received from server.";
			AFRunning = false;
			return;
	}

	// Just prints some log message from server.
	if (i_re.m_log.size())
		AF_LOG << "SERVER: " << i_re.m_log;

	if (i_re.m_id > 0)
	{
		// Render is not connected and a new ID received.
		// This means that server has successfully registered it.
		if (i_render.notConnected())
		{
			i_render.setRegistered(i_re.m_id);
		}
		else if (i_render.getId() != i_re.m_id)
		{
			// Bad case, should not ever happen, try to re-register.
			AF_ERR << "IDs mismatch: this " << i_render.getId() << " != " << i_re.m_id
				   << " new, trying to reconnect...";
			i_render.connectionLost();
			return;
		}
	}

	if (i_re.m_heartbeat_sec)
	{
		if (i_re.m_heartbeat_sec > 0)
			HeartBeatSec = i_re.m_heartbeat_sec;
		else
			HeartBeatSec = AFRENDER::HEARTBEAT_SEC;
		AF_LOG << "Heart beat seconds set to " << HeartBeatSec;
	}

	if (i_re.m_resources_update_period)
	{
		if (i_re.m_resources_update_period > 0)
			ResourcesUpdatePeriod = i_re.m_resources_update_period;
		else
			ResourcesUpdatePeriod = AFRENDER::RESOURCES_UPDATE_PERIOD;
		;
		AF_LOG << "Resources update period set to " << ResourcesUpdatePeriod;
	}

	if (i_re.m_zombie_time)
	{
		if (i_re.m_zombie_time > 0)
			ZombieTime = i_re.m_zombie_time;
		else
			ZombieTime = AFRENDER::ZOMBIETIME;
		AF_LOG << "Zombie time set to " << ZombieTime << " seconds";
	}

	if (i_re.m_exit_no_task_time)
	{
		ExitNoTaskTime = i_re.m_exit_no_task_time;
		AF_LOG << "Exit with no task time set to " << ExitNoTaskTime << " seconds"
			   << ((ExitNoTaskTime > 0) ? "." : " (disabled).");
	}

	// Tasks to execute:
	for (int i = 0; i < i_re.m_tasks.size(); i++)
		i_render.runTask(i_re.m_tasks[i]);

	// Tasks to close:
	for (int i = 0; i < i_re.m_closes.size(); i++)
		i_render.closeTask(i_re.m_closes[i]);

	// Tasks to stop:
	for (int i = 0; i < i_re.m_stops.size(); i++)
		i_render.stopTask(i_re.m_stops[i]);

	// Tasks to outputs:
	for (int i = 0; i < i_re.m_outputs.size(); i++)
		i_render.upTaskOutput(i_re.m_outputs[i]);

	// Listens add:
	for (int i = 0; i < i_re.m_listens_add.size(); i++)
		i_render.listenTask(i_re.m_listens_add[i], true);

	// Listens remove:
	for (int i = 0; i < i_re.m_listens_rem.size(); i++)
		i_render.listenTask(i_re.m_listens_rem[i], false);

	// Instructions:
	if (i_re.m_instruction.size())
	{
		if (i_re.m_instruction == "exit")
		{
			AF_LOG << "Render exit request received.";
			AFRunning = false;
		}
		else if (i_re.m_instruction == "sleep")
		{
			AF_LOG << "Render sleep request received.";
			i_render.wolSleep(i_re.m_command);
		}
		else if (i_re.m_instruction == "launch")
		{
			launchAndExit(i_re.m_command, false);
		}
		else if (i_re.m_instruction == "launch_exit")
		{
			launchAndExit(i_re.m_command, true);
		}
		else if (i_re.m_instruction == "reboot")
		{
			AFRunning = false;
			AF_LOG << "Reboot request, executing command:\n" << af::Environment::getRenderCmdReboot();
			af::launchProgram(af::Environment::getRenderCmdReboot());
		}
		else if (i_re.m_instruction == "shutdown")
		{
			AFRunning = false;
			AF_LOG << "Shutdown request, executing command:\n" << af::Environment::getRenderCmdShutdown();
			af::launchProgram(af::Environment::getRenderCmdShutdown());
		}
	}
}

void launchAndExit(const std::string &i_cmd, bool i_exit)
{
	if (i_exit)
	{
		AF_LOG << "Launching command and exiting:\n" << i_cmd;
		AFRunning = false;
	}
	else
	{
		AF_LOG << "Launching command:\n" << i_cmd;
	}

	af::launchProgram(i_cmd);
}
