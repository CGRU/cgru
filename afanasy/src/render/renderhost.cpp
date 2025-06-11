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

#include "renderhost.h"

#ifdef WINNT
#include <fstream>
#endif

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/taskexec.h"

#include "pyres.h"
#include "res.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

extern bool AFRunning;

extern int ZombieTime;
extern int ExitNoTaskTime;

RenderHost::RenderHost()
	: af::Render(Client::GetEnvironment), m_updateMsgType(af::Msg::TRenderRegister), m_connected(false),
	  m_server_update_time(0), m_no_output_redirection(false)
{
	m_has_tasks_time = time(NULL);

	if (af::Environment::hasArgument("-nor"))
		m_no_output_redirection = true;

	setOnline();

	GetResources(m_hres);

	std::vector<std::string> resclasses = af::Environment::getRenderResClasses();
	for (std::vector<std::string>::const_iterator it = resclasses.begin(); it != resclasses.end(); it++)
	{
		if ((*it).empty())
			continue;
		printf("Adding custom resource meter '%s'\n", (*it).c_str());
		m_pyres.push_back(new PyRes(*it, &m_hres));
	}

#ifdef WINNT
	// Windows Must Die:
	m_windowsmustdie = af::Environment::getRenderWindowsMustDie();
	if (m_windowsmustdie.size())
	{
		printf("Windows Must Die:\n");
		for (int i = 0; i < m_windowsmustdie.size(); i++)
			printf("   %s\n", m_windowsmustdie[i].c_str());
	}
#endif

	af::sleep_msec(100);

	GetResources(m_hres);
	for (int i = 0; i < m_pyres.size(); i++)
		m_pyres[i]->update();

	v_stdOut();
	m_hres.v_stdOut(true);
}

RenderHost::~RenderHost()
{
	FreeResources();

	// Delete custom python resources:
	for (int i = 0; i < m_pyres.size(); i++)
		if (m_pyres[i])
			delete m_pyres[i];

	// Send deregister message if connected:
	if (m_connected)
	{
		af::Msg msg(af::Msg::TRenderDeregister, getId());
		bool ok;
		af::Msg *answer = af::sendToServer(&msg, ok, af::VerboseOn);
		if (answer)
			delete answer;
		m_connected = false;
	}

	// Delete all tasks:
	for (std::vector<TaskProcess *>::iterator it = m_taskprocesses.begin(); it != m_taskprocesses.end();)
	{
		delete *it;
		it = m_taskprocesses.erase(it);
	}
}

RenderHost *RenderHost::getInstance()
{
	// Does not return a reference, although sometimes recommanded for a
	// singleton, because we need to control when it is destroyed.
	static RenderHost *ms_obj = new RenderHost();
	return ms_obj;
}

void RenderHost::connectionEstablished()

{
	if (m_connected)
	{
		m_server_update_time = time(NULL);
	}
	else
	{
		AF_LOG << "Reconnected to the server";
		setUpdateMsgType(af::Msg::TRenderUpdate);
	}
}

void RenderHost::setRegistered(int i_id)
{
	m_connected = true;
	m_id = i_id;
	AF_LOG << "Render registered.";
	RenderHost::connectionEstablished();
}

void RenderHost::serverUpdateFailed()
{
	if (m_connected == false)
	{
		return;
	}

	if (false == AFRunning)
	{
		return;
	}

	AF_LOG << "Failed to connect to server."
		   << " Last success connect time: " << af::time2str(m_server_update_time);
	AF_LOG << "Last connect was: " << (time(NULL) - m_server_update_time) << " seconds ago"
		   << ", Zombie time: " << ZombieTime << "s";

	if (time(NULL) >= (m_server_update_time + ZombieTime))
	{
		connectionLost();
	}
}

void RenderHost::connectionLost()
{
	if (m_connected == false)
		return;

	m_connected = false;

	// Begin to try to register again:
	setUpdateMsgType(af::Msg::TRenderRegister);

	AF_WARN << "Render connection lost, trying to reconnect...";

	if (m_taskprocesses.size())
		AF_LOG << m_taskprocesses.size() << " task(s) are still running.";
}

void RenderHost::setUpdateMsgType(int i_type) { m_updateMsgType = i_type; }

void RenderHost::refreshTasks()
{
	if (false == AFRunning)
		return;

	// Refresh tasks:
	for (int t = 0; t < m_taskprocesses.size(); t++)
	{
		m_taskprocesses[t]->refresh();
	}

	// Check exit with no task time:
	if (m_taskprocesses.size())
	{
		m_has_tasks_time = time(NULL);
	}
	else if ((ExitNoTaskTime > 0) && ((time(NULL) - m_has_tasks_time) >= ExitNoTaskTime))
	{
		AF_LOG << "No tasks for " << ExitNoTaskTime << " seconds.";
		AFRunning = false;
	}

	// Remove zombies:
	for (std::vector<TaskProcess *>::iterator it = m_taskprocesses.begin(); it != m_taskprocesses.end();)
	{
		if ((*it)->isZombie())
		{
			delete *it;
			it = m_taskprocesses.erase(it);
		}
		else
			it++;
	}
}

void RenderHost::getResources()
{
	// Do this every update time, but not the first time, as at the begininng resources are already updated
	static bool first_time = true;
	if (first_time)
	{
		first_time = false;
		return;
	}

	GetResources(m_hres);

	for (int i = 0; i < m_pyres.size(); i++)
		m_pyres[i]->update();

	// hres.stdOut();
	m_up.setResources(&m_hres);

	std::ostringstream str;
	m_hres.jsonWrite(str);
	m_resources_string = str.str();
}

af::Msg *RenderHost::updateServer()
{
	if (false == AFRunning)
		return NULL;

	m_up.setId(getId());

	af::Msg *msg;

#ifdef AFOUTPUT
	AF_LOG << "Tasks size = " << m_taskprocesses.size();
#endif

	if (m_updateMsgType == af::Msg::TRenderRegister)
	{
		m_tasks.clear();

		// Fill tasksexecs array in parent class.
		// This only needed on register to reconnect running tasks if any.
		for (int i = 0; i < m_taskprocesses.size(); i++)
			m_tasks.push_back(m_taskprocesses[i]->getTaskExec());

		msg = new af::Msg(m_updateMsgType, this);

		m_tasks.clear();
	}
	else
	{
#ifdef AFOUTPUT
		AF_LOG << m_up;
#endif
		msg = new af::Msg(m_updateMsgType, &m_up);
	}

#ifdef AFOUTPUT
	AF_LOG << " <<< " << msg;
#endif

	bool ok;
	af::Msg *server_answer =
		af::sendToServer(msg, ok, msg->type() == af::Msg::TRenderRegister ? af::VerboseOff : af::VerboseOn);

	if (ok)
		connectionEstablished();
	else
		serverUpdateFailed();

	delete msg;

	m_up.clear();

	return server_answer;
}

#ifdef WINNT
void RenderHost::windowsMustDie()
{
	// Windows Must Die:
	AFINFO("RenderHost::windowsMustDie():");
	for (int i = 0; i < m_windowsmustdie.size(); i++)
	{
		HWND WINAPI hw = FindWindow(NULL, TEXT(m_windowsmustdie[i].c_str()));
		if (hw != NULL)
		{
			printf("Window must die found:\n%s\n", m_windowsmustdie[i].c_str());
			SendMessage(hw, WM_CLOSE, 0, 0);
		}
	}
}
#endif

void RenderHost::runTask(af::TaskExec *i_task)
{
	// Check that this task is not exist:
	for (int t = 0; t < m_taskprocesses.size(); t++)
	{
		// Skipping closed tasks
		if (m_taskprocesses[t]->isClosed())
			continue;

		if (m_taskprocesses[t]->getTaskExec()->equals(*i_task))
		{
			AF_ERR << "Render asked to run task that is active: " << i_task;
			return;
		}
	}

	m_taskprocesses.push_back(new TaskProcess(i_task, this));
}

void RenderHost::stopTask(const af::MCTaskPos &i_taskpos)
{
	for (int t = 0; t < m_taskprocesses.size(); t++)
	{
		if (m_taskprocesses[t]->is(i_taskpos))
		{
			m_taskprocesses[t]->stop();
			return;
		}
	}

	AF_ERR << "RenderHost::stopTask: No such task: " << i_taskpos << " (now running "
		   << int(m_taskprocesses.size()) << " tasks).";
}

void RenderHost::closeTask(const af::MCTaskPos &i_taskpos)
{
	for (int t = 0; t < m_taskprocesses.size(); t++)
	{
		if (m_taskprocesses[t]->is(i_taskpos))
		{
			m_taskprocesses[t]->close();
			return;
		}
	}
	AFERRAR("RenderHost::closeTask: %d tasks, no such task:", int(m_taskprocesses.size()))
	i_taskpos.v_stdOut();
}

void RenderHost::upTaskOutput(const af::MCTaskPos &i_taskpos)
{
	std::string str;
	for (int t = 0; t < m_taskprocesses.size(); t++)
	{
		if (m_taskprocesses[t]->is(i_taskpos))
		{
			str = m_taskprocesses[t]->getOutput();
			break;
		}
	}

	if (str.size() == 0)
	{
		str = "Render has no task:";
		str += i_taskpos.v_generateInfoString();
	}

	m_up.addTaskOutput(i_taskpos, str);
}

void RenderHost::listenTask(const af::MCTaskPos &i_tp, bool i_subscribe)
{
	for (int t = 0; t < m_taskprocesses.size(); t++)
	{
		if (m_taskprocesses[t]->is(i_tp))
		{
			m_taskprocesses[t]->listenOutput(i_subscribe);
			break;
		}
	}
}

void RenderHost::wolSleep(const std::string &i_str)
{
	af::Service service(af::Environment::getSysWolService(), "SLEEP", i_str);
	std::string cmd = service.getCommand();
	printf("Sleep request, executing command:\n%s\n", cmd.c_str());
	af::launchProgram(cmd);
}
