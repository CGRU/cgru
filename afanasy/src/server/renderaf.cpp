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

/*
	This is a server side of an Afanasy render.
*/
#include "renderaf.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/msgqueue.h"
#include "../libafanasy/regexp.h"

#include "action.h"
#include "afcommon.h"
#include "jobcontainer.h"
#include "monitorcontainer.h"
#include "poolscontainer.h"
#include "rendercontainer.h"
#include "sysjob.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

RenderContainer * RenderAf::ms_renders = NULL;

RenderAf::RenderAf( af::Msg * msg):
	af::Render( msg),
	AfNodeFarm(this, this, AfNodeFarm::TRenderer, NULL)
{
	initDefaultValues();
}

RenderAf::RenderAf( const std::string & i_store_dir):
	af::Render(),
	AfNodeFarm(this, this, AfNodeFarm::TRenderer, NULL, i_store_dir)
{
	initDefaultValues();

	int size;
	char * data = af::fileRead( getStoreFile(), &size);
	if( data == NULL ) return;

	rapidjson::Document document;
	char * res = af::jsonParseData( document, data, size);
	if( res == NULL )
	{
		delete [] data;
		return;
	}

	if( jsonRead( document))
		setStoredOk();

	delete [] res;
	delete [] data;

	// This render came from store on server start, it can't be online or busy
	setOffline();
	setBusy( false);
}

void RenderAf::initDefaultValues()
{
	m_parent = NULL;

	setBusy( false);
	setWOLFalling( false);
	setWOLSleeping( false);
	setWOLWaking( false);
}

RenderAf::~RenderAf()
{
}

void RenderAf::setRegistered(PoolsContainer * i_pools)
{
	findPool(i_pools);

	getFarmHost();

	if( isFromStore())
	{
		appendLog("Initialized from store.");
	}
	else
	{
		std::string log = "Registered";

		if (m_parent->newNimby())
		{
			setNimby();
			log += " nimby";
		}
		if (m_parent->newPaused())
		{
			setPaused(true);
			log += " paused";
		}

		setStoreDir( AFCommon::getStoreDirRender( *this));
		store();

		appendLog( log + ".");
	}

	af::Client::setRegisterTime();

	m_task_start_finish_time = 0;
	m_wol_operation_time = 0;
	m_idle_time = time(NULL);
	m_busy_time = m_idle_time;
}

void RenderAf::findPool(PoolsContainer * i_pools)
{
	if (m_parent)
	{
		AF_ERR << "Render '" << getName() << "' already in pool '" + m_parent->getName() + "'";
		return;
	}

	if (m_pool.empty())
	{
		i_pools->assignRender(this);
		return;
	}

	m_parent = i_pools->getPool(m_pool);
	if (NULL == m_parent)
	{
		AF_ERR << "Render pool '" + m_pool + "' does not exist.";
		i_pools->assignRender(this);
		return;
	}

	m_parent->addRender(this);
}

void RenderAf::setPool(PoolSrv * i_pool)
{
	m_pool = i_pool->getName();
	m_parent = i_pool;
}

void RenderAf::offline( JobContainer * jobs, uint32_t updateTaskState, MonitorContainer * monitoring, bool toZombie )
{
	setOffline();
	setBusy( false);
	if( isWOLFalling())
	{
		setWOLFalling( false);
		setWOLSleeping( true);
	}

	if( jobs && updateTaskState)
		ejectTasks( jobs, monitoring, updateTaskState);

	// There is need to send pending tasks to offline render.
	m_re.clearTaskExecs();

	appendLog( m_hres.v_generateInfoString());

	if( toZombie )
	{
		AFCommon::QueueLog("Render Deleting: " + v_generateInfoString( false));
		appendLog("Waiting for deletion.");
		if (m_parent)
		{
			m_parent->removeRender(this);
			if (monitoring) monitoring->addEvent( af::Monitor::EVT_pools_change, m_parent->getId());
		}
		setZombie();
		if (monitoring) monitoring->addEvent( af::Monitor::EVT_renders_del, m_id);
	}
	else
	{
		AFCommon::QueueLog("Render Offline: " + v_generateInfoString( false));
		appendLog("Offline.");
		m_time_launch = 0;
		if( monitoring ) monitoring->addEvent( af::Monitor::EVT_renders_change, m_id);
		store();
	}
}

af::Msg * RenderAf::update( const af::RenderUpdate & i_up)
{
	updateTime();

	if( i_up.hasResources())
		m_hres.copy( *i_up.getResources());

	if( i_up.m_taskups.size())
		for( int i = 0; i < i_up.m_taskups.size(); i++)
			if( i_up.m_taskups[i]->getStatus() > af::TaskExec::UPWarning )
				m_re.addTaskClose( af::MCTaskPos( *i_up.m_taskups[i]));

	if( m_re.isEmpty())
	{
		// If there is no new events just return its id back.
		return new af::Msg( af::Msg::TRenderId, getId());
	}

	af::Msg * msg = new af::Msg( af::Msg::TRenderEvents, &m_re);

	m_re.clear();

	return msg;
}

void RenderAf::online( RenderAf * render, JobContainer * i_jobs, MonitorContainer * monitoring)
{
	if( isOnline())
	{
		AF_ERR << "Render is already online.";
		return;
	}

	// Update some attributes to make it online:
	m_task_start_finish_time = 0;
	m_idle_time = time(NULL);
	m_busy_time = m_idle_time;
	setBusy(false);
	setWOLSleeping(false);
	setWOLWaking(false);
	setWOLFalling(false);
	updateTime();
	setOnline();

	// Grab some attributes from an incoming render data:
	m_os = render->m_os;
	m_time_launch = render->m_time_launch;
	m_engine = render->m_engine;
	m_address.copy(render->getAddress());
	grabNetIFs(render->m_netIFs);
	getFarmHost(&render->m_host);
	m_hres.copy(render->getHostRes());


	// Reconnect tasks if any:
	std::list<af::TaskExec*>::iterator it;
	for( it = render->m_tasks.begin() ; it != render->m_tasks.end() ; ++it)
	{
		i_jobs->reconnectTask( *it, *this, monitoring);
	}
	// Job::reconnectTask took the ownership of the taskexecs, so we prevent
	// them from being cleaned by render's dtor:
	render->m_tasks.clear();
	


	std::string str = "Online '" + m_engine + "'.";
	appendLog( str);

	if( monitoring )
		monitoring->addEvent( af::Monitor::EVT_renders_change, m_id);

	store();
}

void RenderAf::deregister( JobContainer * jobs, MonitorContainer * monitoring )
{
	if( isOffline())
	{
		appendLog("Render deregister request - offline already.");
		return;
	}
	appendLog("Render deregister request.");
	offline( jobs, af::TaskExec::UPRenderDeregister, monitoring);
}

void RenderAf::setTask( af::TaskExec *taskexec, MonitorContainer * monitoring, bool start)
{
  if( isOffline())
	{
		AF_ERR << "Render is offline.";
		return;
	}
	if( taskexec == NULL)
	{
		AF_ERR << "taskexec == NULL.";
		return;
	}

	addTask( taskexec);

	if( monitoring ) monitoring->addEvent( af::Monitor::EVT_renders_change, m_id);

	if( start)
	{
		// Add exec pointer to events,
		// so on refresh, render will receive a task to execute.
		m_re.addTaskExec( taskexec);

		// Log:
		std::string str = "Starting task: ";
		str += taskexec->v_generateInfoString( false);
		appendTasksLog( str);
	}
	else
	{
		// This is multihost task.
		std::string str = "Captured by task: ";
		str += taskexec->v_generateInfoString( false);
		appendTasksLog( str);
	}
}

// Needed for mulihost tasks
void RenderAf::startTask( af::TaskExec *taskexec)
{
	if( isOffline())
	{
		AF_ERR << "Render is offline.";
		return;
	}
	for( std::list<af::TaskExec*>::const_iterator it = m_tasks.begin(); it != m_tasks.end(); it++)
	{
		if( taskexec != *it) continue;

		m_re.m_tasks.push_back( taskexec);

		std::string str = "Starting service: ";
		str += taskexec->v_generateInfoString( false);
		appendLog( str);

		return;
	}

	AF_ERR << "No such task.";
	taskexec->v_stdOut( false);
}

void RenderAf::v_action( Action & i_action)
{
	const JSON & operation = (*i_action.data)["operation"];
	if( operation.IsObject())
	{
		std::string type;
		af::jr_string("type", type, operation);
		if( type == "exit")
		{
			if( false == isOnline()) return;
			appendLog("Exit by " + i_action.author);
			exitClient("exit", i_action.jobs, i_action.monitors);
			return;
		}
		else if( type == "launch_cmd")
		{
			if( false == isOnline()) return;
			std::string cmd;
			if( af::jr_string("cmd", cmd, operation))
			{
				bool exit = false;
				af::jr_bool("exit", exit, operation);
				if( exit )
					appendLog("Launch command and exit request by " + i_action.author + "\n" + cmd);
				else
					appendLog("Launch command request by " + i_action.author + "\n" + cmd);
				launchAndExit( cmd, exit, i_action.jobs, i_action.monitors);
			}
			else
			{
				appendLog("Launch command exit request by " + i_action.author + "has no 'cmd'");
			}
			return;
		}
		else if( type == "eject_tasks")
		{
			if( false == isBusy()) return;
			appendLog("Task(s) ejected by " + i_action.author);
			ejectTasks( i_action.jobs, i_action.monitors, af::TaskExec::UPEject);
			return;
		}
		else if( type == "eject_tasks_keep_my")
		{
			if( false == isBusy()) return;
			appendLog("Task(s) ejected keeping own by " + i_action.author);
			ejectTasks( i_action.jobs, i_action.monitors, af::TaskExec::UPEject, &i_action.user_name);
			return;
		}
		else if( type == "delete")
		{
			if(isOnline())
			{
				i_action.answer_kind = "error";
				i_action.answer = "Can`t delete online render.";
				return;
			}
			appendLog( std::string("Deleted by ") + i_action.author);
			offline( NULL, 0, i_action.monitors, true);
			return;
		}
		else if( type == "reboot")
		{
			if( false == isOnline() ) return;
			appendLog( std::string("Reboot computer by ") + i_action.author);
			exitClient("reboot", i_action.jobs, i_action.monitors);
			return;
		}
		else if( type == "shutdown")
		{
			if( false == isOnline() ) return;
			appendLog( std::string("Shutdown computer by ") + i_action.author);
			exitClient("shutdown", i_action.jobs, i_action.monitors);
			return;
		}
		else if( type == "wol_sleep")
			wolSleep( i_action.monitors);
		else if( type == "wol_wake")
			wolWake( i_action.monitors);
		else if (type == "farm")
		{
			if (false == actionFarm(i_action))
				return;
		}
		else if (type == "set_pool")
		{
			std::string pool_name;
			af::jr_string("name", pool_name, operation);
			actionSetPool(pool_name, i_action);
		}
		else if (type == "reassign_pool")
		{
			actionReassignPool(i_action);
		}
		else
		{
			appendLog("Unknown operation \"" + type + "\" by " + i_action.author);
			i_action.answer_kind = "error";
			i_action.answer = "Unknown operation: " + type;
			return;
		}

		appendLog("Operation \"" + type + "\" by " + i_action.author);
		i_action.monitors->addEvent( af::Monitor::EVT_renders_change, m_id);
		store();
		return;
	}

	const JSON & params = (*i_action.data)["params"];
	if( params.IsObject())
		jsonRead( params, &i_action.log);

	if( i_action.log.size() )
	{
		store();
		i_action.monitors->addEvent( af::Monitor::EVT_renders_change, m_id);
	}
}

void RenderAf::actionSetPool(const std::string & i_pool_name, Action & i_action)
{
	if (m_pool == i_pool_name)
	{
		i_action.answer_kind = "error";
		i_action.answer = "Render '" + getName() + "' already in a poll '" + i_pool_name + "'.";
		return;
	}

	PoolSrv * pool = i_action.pools->getPool(i_pool_name);
	if (NULL == pool)
	{
		i_action.answer_kind = "error";
		i_action.answer = "Pool '" + i_pool_name + "' does not exist.";
		return;
	}

	if (pool->hasRender(this))
	{
		i_action.answer_kind = "error";
		i_action.answer = "Pool '" + pool->getName() + "' already has a render '" + getName() + "'.";
		return;
	}

	if (m_parent)
	{
		m_parent->removeRender(this);
		i_action.monitors->addEvent(af::Monitor::EVT_pools_change, m_parent->getId());
	}

	m_pool = i_pool_name;
	pool->addRender(this);
	m_parent = pool;

	i_action.monitors->addEvent(af::Monitor::EVT_pools_change, m_parent->getId());
}

void RenderAf::actionReassignPool(Action & i_action)
{
	if (m_parent)
	{
		m_parent->removeRender(this);
		i_action.monitors->addEvent(af::Monitor::EVT_pools_change, m_parent->getId());
	}

	i_action.pools->assignRender(this);

	i_action.monitors->addEvent(af::Monitor::EVT_pools_change, m_parent->getId());
}

void RenderAf::ejectTasks( JobContainer * jobs, MonitorContainer * monitoring, uint32_t upstatus, const std::string * i_keeptasks_username )
{
	if( m_tasks.size() < 1)
		return;

	std::list<int>id_jobs;
	std::list<int>id_blocks;
	std::list<int>id_tasks;
	std::list<int>numbers;
	for( std::list<af::TaskExec*>::const_iterator it = m_tasks.begin(); it != m_tasks.end(); it++)
	{
		 // Skip if username to keep tasks provided:
		 if( i_keeptasks_username != NULL )
			  if( *i_keeptasks_username == (*it)->getUserName())
					continue;
		id_jobs.push_back( (*it)->getJobId());
		id_blocks.push_back( (*it)->getBlockNum());
		id_tasks.push_back( (*it)->getTaskNum());
		numbers.push_back( (*it)->getNumber());
		appendLog( std::string("Ejecting task: ") + (*it)->v_generateInfoString( false));
	}

	JobContainerIt jobsIt( jobs);
	std::list<int>::const_iterator jIt = id_jobs.begin();
	std::list<int>::const_iterator bIt = id_blocks.begin();
	std::list<int>::const_iterator tIt = id_tasks.begin();
	std::list<int>::const_iterator nIt = numbers.begin();
	std::list<int>::const_iterator end = numbers.end();
	for( ; nIt != end; jIt++, bIt++, tIt++, nIt++)
	{
		JobAf* job = jobsIt.getJob( *jIt);
		if( job != NULL )
		{
			af::MCTaskUp taskup( m_id, *jIt, *bIt, *tIt, *nIt, upstatus);
			job->v_updateTaskState( taskup, ms_renders, monitoring);
		}
	}
}

void RenderAf::exitClient( const std::string & i_type, JobContainer * i_jobs, MonitorContainer * i_monitoring)
{
	if( false == isOnline() ) return;

	m_re.m_instruction = i_type;
}

void RenderAf::launchAndExit( const std::string & i_cmd, bool i_exit, JobContainer * i_jobs, MonitorContainer * i_monitoring)
{
	if( false == isOnline() ) return;

	m_re.m_instruction = ( i_exit ? "launch_exit" : "launch");
	m_re.m_command = i_cmd;
}

void RenderAf::wolSleep( MonitorContainer * monitoring)
{
	if( isWOLSleeping())
	{
		appendLog("Render is already sleeping.");
		return;
	}
	if( isWOLWaking())
	{
		appendLog("Can't sleep waking up render.");
		return;
	}
	if( isOffline())
	{
		appendLog("Can't sleep offline render.");
		return;
	}
	if( isBusy())
	{
		appendLog("Can't perform Wake-On-Line operations. Render is busy.");
		return;
	}
	if( m_netIFs.size() < 1)
	{
		appendLog("Can't perform Wake-On-Line operations. No network interfaces information.");
		return;
	}

	setWOLFalling( true);
	appendLog("Sending WOL sleep request.");
	m_wol_operation_time = time( NULL);
	store();
	if( monitoring ) monitoring->addEvent( af::Monitor::EVT_renders_change, m_id);

	std::ostringstream str;
	v_jsonWrite( str, af::Msg::TRendersList);

	m_re.m_instruction = "sleep";
	m_re.m_command = str.str();
}

void RenderAf::wolWake(  MonitorContainer * i_monitoring, const std::string & i_msg)
{
	if( i_msg.size())
		appendLog( i_msg);

	if( isOnline())
	{
		appendLog("Can't wake up online render.");
		return;
	}
	if( isWOLFalling())
	{
		appendLog("Can't wake up render which is just falling a sleep.");
		return;
	}

	if( m_netIFs.size() < 1)
	{
		appendLog("Can't perform Wake-On-Line operations. No network interfaces information.");
		return;
	}

	appendLog("Sending WOL wake request.");
	setWOLWaking( true);
	m_wol_operation_time = time( NULL);
	store();
	if( i_monitoring ) i_monitoring->addEvent( af::Monitor::EVT_renders_change, m_id);

	std::ostringstream str;
	v_jsonWrite( str, af::Msg::TRendersList);

	SysJob::AddWOLCommand( str.str(), "", m_name, m_name);
}

void RenderAf::stopTask( int jobid, int blocknum, int tasknum, int number)
{
	//printf("RenderAf::stopTask: j%d b%d t%d n%d\n", jobid, blocknum, tasknum, number);
	if( isOffline()) return;

	m_re.addTaskStop( af::MCTaskPos( jobid, blocknum, tasknum, number));
}

void RenderAf::taskFinished( const af::TaskExec * taskexec, MonitorContainer * monitoring)
{
	removeTask( taskexec);

	if( taskexec->getNumber())
	{
		std::string str = "Finished service: ";
		str += taskexec->v_generateInfoString( false);
		appendTasksLog( str);
	}
	else
	{
		std::string str = "Finished task: ";
		str += taskexec->v_generateInfoString( false);
		appendTasksLog( str);
	}

	if( monitoring ) monitoring->addEvent( af::Monitor::EVT_renders_change, m_id);
}

void RenderAf::addTask( af::TaskExec * taskexec)
{
	// If render was not busy it has become busy now
	if( false == isBusy())
	{
		setBusy( true);
		m_task_start_finish_time = time( NULL);
		store();
	}

	#ifdef AFOUTPUT
	AF_DEBUG << *taskexec;
	#endif

	m_tasks.push_back( taskexec);

	m_capacity_used += taskexec->getCapResult();

	// Just check capacity:
	if ((getCapacity() >= 0) && (m_capacity_used > getCapacity()))
		AF_ERR << "Capacity_used > max capacity (" << m_capacity_used << " > " << getCapacity() << ")";

	//farmTaskAcuire( taskexec->getServiceType());
}

void RenderAf::removeTask( const af::TaskExec * i_exec)
{
	// Do not set free status here, even if this task was last.
	// May it will take another task in this run cycle

	// We should remove af::TaskExec poiter from everywhere!
	// As this af::TaskExec will be deleted by TaskRun very soon.

	// Remove exec pointer:
	for( std::list<af::TaskExec*>::iterator it = m_tasks.begin(); it != m_tasks.end(); it++)
	{
		if( *it == i_exec)
		{
			it = m_tasks.erase( it);
		}
	}

	// Remove exec pointer from events:
	m_re.remTaskExec( i_exec);

	if( m_capacity_used < i_exec->getCapResult())
	{
		AF_ERR << "Capacity_used < getCapResult() (" << m_capacity_used << " < " << i_exec->getCapResult() << ")";
		m_capacity_used = 0;
	}
	else m_capacity_used -= i_exec->getCapResult();

	//farmTaskRelease( taskexec->getServiceType());
}

void RenderAf::v_refresh( time_t currentTime,  AfContainer * pointer, MonitorContainer * monitoring)
{
	if( isLocked() ) return;

	JobContainer * jobs = (JobContainer*)pointer;


	if ((NULL == m_parent) && isOnline())
	{
		AF_ERR << "Render '" + getName() + "' has no pool.";
		offline(jobs, af::TaskExec::UPRenderZombie, monitoring);
		appendLog("Has no pool");
		monitoring->addEvent(af::Monitor::EVT_renders_change, m_id);
	}

	if( isOnline() && (getTimeUpdate() < (currentTime - af::Environment::getRenderZombieTime())))
	{
		appendLog( std::string("ZOMBIETIME: ") + af::itos(af::Environment::getRenderZombieTime()) + " seconds.");
		AFCommon::QueueLog( std::string("Render: \"") + getName() + "\" - ZOMBIETIME");
		offline( jobs, af::TaskExec::UPRenderZombie, monitoring);
		return;
	}

	// Later auto nimby operations not needed if render is not online:
	if( isOffline())
	{
		m_busy_time = currentTime;
		m_idle_time = currentTime;
		return;
	}

	// Update busy with no task time:
	if(( isFree() == false ) || isBusy()) // already nimby or has task(s)
	{
		m_busy_time = currentTime;
	}
	else
	{
		int cpu=0,mem=0,swp=0,hgb=0,hio=0,net=0;

		// CPU % busy:
		if ((m_parent->get_busy_cpu() <= 0) ||
			((100 - m_hres.cpu_idle) < m_parent->get_busy_cpu()))
			cpu = 1;

		// Mem % used:
		if ((m_hres.mem_total_mb <= 0) || (m_parent->get_busy_mem() <= 0) ||
			((100 * (m_hres.mem_total_mb - m_hres.mem_free_mb) / m_hres.mem_total_mb) < m_parent->get_busy_mem()))
			mem = 1;

		// Swap % used:
		if ((m_hres.swap_total_mb <= 0) || ((m_parent->get_busy_swp() <= 0) ||
			(100 * m_hres.swap_used_mb / m_hres.swap_total_mb) < m_parent->get_busy_swp()))
			swp = 1;

		// Hdd free GB:
		if ((m_hres.hdd_total_gb <= 0) || (m_parent->get_busy_hddgb() <= 0) ||
			(m_hres.hdd_free_gb > m_parent->get_busy_hddgb()))
			hgb = 1;

		// Hdd I/O %:
		if ((m_parent->get_busy_hddio() <= 0) ||
			(m_hres.hdd_busy < m_parent->get_busy_hddio()))
			hio = 1;

		// Net Mb/s:
		if ((m_parent->get_busy_netmbs() <= 0) ||
			(m_hres.net_recv_kbsec + m_hres.net_send_kbsec < (1024 * m_parent->get_busy_netmbs())))
			net = 1;

		// Render will be treated as 'not busy' if all params are 'not busy'
		if (cpu & mem & swp & hio & hgb & net)
		{
			m_busy_time = currentTime;
		}
		else if ((m_parent->get_busy_nimby_time() > 0) && (currentTime - m_busy_time > m_parent->get_busy_nimby_time()))
		{
		// Automatic Nimby ON:
			std::string log("Automatic Nimby: ");
			log += "\n Busy since: " + af::time2str (m_busy_time);// + " CPU >= " + af::itos (m_parent->get_busy_cpu) + "%";
			log += "\n Nimby busy free time = " + af::time2strHMS (m_parent->get_busy_nimby_time(), true);
			appendLog (log);
			setNIMBY();
			monitoring->addEvent (af::Monitor::EVT_renders_change, m_id);
			store();
		}
	}

	// Update idle time:
	if( isBusy())
	{
		m_idle_time = currentTime;
	}
	else if((m_parent->get_idle_cpu()    <= 0) &&
			(m_parent->get_idle_mem()    <= 0) &&
			(m_parent->get_idle_swp()    <= 0) &&
			(m_parent->get_idle_hddgb()  <= 0) &&
			(m_parent->get_idle_hddio()  <= 0) &&
			(m_parent->get_idle_netmbs() <= 0))
	{
		// If all params are 'off' there is no 'idle':
		m_idle_time = currentTime;
	}
	else
	{
		int cpu=0,mem=0,swp=0,hgb=0,hio=0,net=0;

		// CPU % busy:
		if ((m_parent->get_idle_cpu() > 0) &&
			((100 - m_hres.cpu_idle) > m_parent->get_idle_cpu()))
			cpu = 1;

		// Mem % used:
		if ((m_hres.mem_total_mb > 0) && (m_parent->get_idle_mem() > 0) &&
			((100 * (m_hres.mem_total_mb - m_hres.mem_free_mb) / m_hres.mem_total_mb) > m_parent->get_idle_mem()))
			mem = 1;

		// Swap % used:
		if ((m_hres.swap_total_mb) && (m_parent->get_idle_swp() > 0) &&
			((100 * m_hres.swap_used_mb / m_hres.swap_total_mb) > m_parent->get_idle_swp()))
			swp = 1;

		// Hdd free GB:
		if ((m_hres.hdd_total_gb > 0) && (m_parent->get_idle_hddgb() > 0) &&
			(m_hres.hdd_free_gb < m_parent->get_idle_hddgb()))
			hgb = 1;

		// Hdd I/O %:
		if ((m_parent->get_idle_hddio() > 0) &&
			(m_hres.hdd_busy > m_parent->get_idle_hddio()))
			hio = 1;

		// Net Mb/s:
		if ((m_parent->get_idle_netmbs() > 0) &&
			(m_hres.net_recv_kbsec + m_hres.net_send_kbsec > (1024 * m_parent->get_idle_netmbs())))
			net = 1;

		// it will be treated as 'not idle' any param is 'not idle'
		if( cpu | mem | swp | hio | hgb | net )
		{
			m_idle_time = currentTime;
		}
		else 
		{
			// Automatic WOL sleep:
			if ((m_parent->get_idle_wolsleep_time() > 0) && isOnline() && (isWOLSleeping() == false) && (isWOLFalling() == false)
				&& (currentTime - m_idle_time > m_parent->get_idle_wolsleep_time()))
			{
				std::string log("Automatic WOL Sleep: ");
				log += "\n Idle since: " + af::time2str(m_idle_time);
				log += "\n WOL idle sleep time = " + af::time2strHMS(m_parent->get_idle_wolsleep_time(), true);
				appendLog(log);
				wolSleep(monitoring);
			}

			// Automatic Nimby Free:
			if ((m_parent->get_idle_free_time() > 0) && isOnline() && (isNimby() || isNIMBY())
				&& (currentTime - m_idle_time > m_parent->get_idle_free_time()))
			{
				std::string log("Automatic Nimby Free: ");
				log += "\n Idle since: " + af::time2str(m_idle_time);
				log += "\n Nimby idle free time = " + af::time2strHMS(m_parent->get_idle_free_time(), true );
				appendLog(log);
				setFree();
				monitoring->addEvent(af::Monitor::EVT_renders_change, m_id);
				store();
			}
		}
	}
}

void RenderAf::v_postSolve(time_t i_curtime, MonitorContainer * i_monitoring)
{
	bool changed = false;

	// If render was busy but has no tasks after solve it is not busy now
	// Needed not to reset busy render status if it run one task after other
	if (isBusy() && (m_tasks.size() == 0))
	{
		setBusy(false);
		m_task_start_finish_time = i_curtime;
		changed = true;
	}

	if (changed)
	{
		store();
		if (i_monitoring)
			i_monitoring->addEvent(af::Monitor::EVT_renders_change, m_id);
	}
}

void RenderAf::appendTasksLog( const std::string & message)
{
	while( m_tasks_log.size() > af::Environment::getAfNodeLogLinesMax() ) m_tasks_log.pop_front();
	m_tasks_log.push_back( af::time2str() + " : " + message);
}

af::Msg * RenderAf::writeTasksLog( bool i_binary)
{
	if( false == i_binary )
	{
		if( m_tasks_log.empty())
			return af::jsonMsg("tasks_log", m_name, "No tasks execution.");
		else
			return af::jsonMsg("tasks_log", m_name, m_tasks_log);
	}

	af::Msg * msg = new af::Msg;

	if( m_tasks_log.empty())
		msg->setString("No tasks execution.");
	else
		msg->setStringList( m_tasks_log);

	return msg;
}

void RenderAf::getFarmHost( af::Host * newHost)
{
	// When render becames online it refresh hardware information:
	if( newHost ) m_host.copy( *newHost);
}

void RenderAf::closeLostTask( const af::MCTaskUp &taskup)
{
	RenderContainerIt rIt( ms_renders);
	RenderAf * render = rIt.getRender( taskup.getClientId());
	if( render == NULL)
	{
		std::ostringstream stream;
		stream << "RenderAf::closeLostTask: Render with id=" << taskup.getClientId() << " does not exists.";
		AFCommon::QueueLogError( stream.str());
		return;
	}
	if( render->isOffline()) return;

	std::ostringstream stream;
	stream << "RenderAf::closeLostTask: '" << render->getName() << "': ";
	stream << "[" << taskup.getNumJob() << "][" << taskup.getNumBlock() << "][" << taskup.getNumTask() << "](" << taskup.getNumBlock() << ")";
	AFCommon::QueueLogError( stream.str());

	render->m_re.addTaskClose( taskup);
}

af::Msg * RenderAf::writeFullInfo( bool i_binary) const
{
	if( i_binary )
	{
		af::Msg * o_msg = new af::Msg();

		std::string str = v_generateInfoString( true);
		if( m_custom_data.size())
		str += "\nCustom Data:\n" + m_custom_data;

		o_msg->setString( str);

		return o_msg;
	}

	std::ostringstream str;
	str << "{\"object\":{";

	str << "\"name\":\"" << m_name << "\"";
	str << ",\"id\":" << m_id;

	str << ",\"render\":";
	af::Render::v_jsonWrite( str, af::Msg::TRendersList);

	str << ",\"custom_data\":\"" << m_custom_data << '"';

	if( isOnline())
	{
		str << ",";
		m_hres.jsonWrite( str);
	}
	
	str << "}}";
	return af::jsonMsg( str);
}

int RenderAf::v_calcWeight() const
{
	int weight = Render::v_calcWeight();
	weight += sizeof(RenderAf) - sizeof( Render);
	return weight;
}
