#include "renderaf.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/msgqueue.h"
#include "../libafanasy/msgclasses/mclistenaddress.h"
#include "../libafanasy/farm.h"

#include "../libafsql/dbattr.h"

#include "action.h"
#include "afcommon.h"
#include "jobcontainer.h"
#include "monitorcontainer.h"
#include "rendercontainer.h"
#include "sysjob.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

RenderContainer * RenderAf::ms_renders = NULL;

RenderAf::RenderAf( af::Msg * msg):
	DBRender( msg),
	AfNodeSrv( this)
{
	init();
}

RenderAf::RenderAf( int Id):
	DBRender( Id),
	AfNodeSrv( this)
{
//printf("RenderAf::RenderAf:\n");
//printf("this = %p\n", this);
//	setNode( this);
	AFINFA("RenderAf::RenderAf(%d)", m_id);
	init();
}

void RenderAf::init()
{
	hostname = "no farm host";
	hostdescription = "";
	servicesnum = 0;
	if( m_host.m_capacity == 0 ) m_host.m_capacity = af::Environment::getRenderDefaultCapacity();
	if( m_host.m_max_tasks == 0 ) m_host.m_max_tasks = af::Environment::getRenderDefaultMaxTasks();
	setBusy( false);
	setWOLFalling( false);
	setWOLSleeping( false);
	setWOLWaking( false);
}

RenderAf::~RenderAf()
{
}

void RenderAf::setRegisterTime()
{
	af::Client::setRegisterTime();

	m_task_start_finish_time = 0;
	m_wol_operation_time = 0;
	m_idle_time = time(NULL);
	m_busy_time = m_idle_time;

	if( isOnline()) appendLog("Registered online.");
	else appendLog("Registered offline.");
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

	if( jobs && updateTaskState) ejectTasks( jobs, monitoring, updateTaskState);

	appendLog( m_hres.v_generateInfoString());

	if( toZombie )
	{
		AFCommon::QueueLog("Render Deleting: " + v_generateInfoString( false));
		appendLog("Waiting for deletion.");
		v_setZombie();
	  AFCommon::saveLog( getLog(), af::Environment::getRendersLogsDir(), m_name, af::Environment::getAfNodeLogsRotate());
		if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersDel, m_id);
	}
	else
	{
		AFCommon::QueueLog("Render Offline: " + v_generateInfoString( false));
		appendLog("Offline.");
		m_time_launch = 0;
		if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersChanged, m_id);
		AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
	}
}

bool RenderAf::update( const af::Render * render)
{
	if( isOffline()) return false;
	if( render == NULL )
	{
		AFERROR("Render::update( Render * render): render == NULL")
		return false;
	}

	m_hres.copy( render->getHostRes());

	updateTime();
	return true;
}

bool RenderAf::online( RenderAf * render, MonitorContainer * monitoring)
{
	if( isOnline())
	{
		AFERROR("RenderAf::online: Render is already online.")
		return false;
	}
	m_idle_time = time( NULL);
	m_busy_time = m_idle_time;
	setBusy( false);
	setWOLSleeping( false);
	setWOLWaking( false);
	setWOLFalling( false);
	m_address.copy( render->getAddress());
	grabNetIFs( render->m_netIFs);
	m_time_launch = render->m_time_launch;
	m_version = render->m_version;
	m_task_start_finish_time = 0;
	getFarmHost( &render->m_host);
	setOnline();
	update( render);
	std::string str = "Online v'" + m_version + "'.";
	appendLog( str);
	if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersChanged, m_id);
	AFCommon::QueueDBUpdateItem( this);
	return true;
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
		AFERROR("RenderAf::setTask: Render is offline.")
		return;
	}
	if( taskexec == NULL)
	{
		AFERROR("RenderAf::setTask: taskexec == NULL.")
		return;
	}

	addTask( taskexec);
	addService( taskexec->getServiceType());
	if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersChanged, m_id);

	if( start)
	{
		af::Msg* msg = new af::Msg( af::Msg::TTask, taskexec);
		msg->setAddress( this);
		AFCommon::QueueMsgDispatch( msg);
		std::string str = "Starting task: ";
		str += taskexec->v_generateInfoString( false);
		appendTasksLog( str);
	}
	else
	{
		std::string str = "Captured by task: ";
		str += taskexec->v_generateInfoString( false);
		appendTasksLog( str);
	}
}

void RenderAf::startTask( af::TaskExec *taskexec)
{
	if( isOffline())
	{
		AFERROR("RenderAf::startTask: Render is offline.")
		return;
	}
	for( std::list<af::TaskExec*>::const_iterator it = m_tasks.begin(); it != m_tasks.end(); it++)
	{
		if( taskexec != *it) continue;

		af::Msg* msg = new af::Msg( af::Msg::TTask, taskexec);
		msg->setAddress( this);
		AFCommon::QueueMsgDispatch( msg);

		std::string str = "Starting service: ";
		str += taskexec->v_generateInfoString( false);
		appendLog( str);

		return;
	}

	AFERROR("RenderAf::startTask: No such task.")
	taskexec->v_stdOut( false);
}

void RenderAf::v_priorityChanged( MonitorContainer * i_monitoring) { ms_renders->sortPriority( this);}

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
			exitClient( af::Msg::TClientExitRequest, i_action.jobs, i_action.monitors);
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
			if( isOnline() ) return;
			appendLog( std::string("Deleted by ") + i_action.author);
			offline( NULL, 0, i_action.monitors, true);
			AFCommon::QueueDBDelItem( this);
			return;
		}
		else if( type == "reboot")
		{
			if( false == isOnline() ) return;
			appendLog( std::string("Reboot computer by ") + i_action.author);
			exitClient( af::Msg::TClientRebootRequest, i_action.jobs, i_action.monitors);
			return;
		}
		else if( type == "shutdown")
		{
			if( false == isOnline() ) return;
			appendLog( std::string("Shutdown computer by ") + i_action.author);
			exitClient( af::Msg::TClientShutdownRequest, i_action.jobs, i_action.monitors);
			return;
		}
		else if( type == "wol_sleep")
			wolSleep( i_action.monitors);
		else if( type == "wol_wake")
			wolWake( i_action.monitors);
		else if( type == "service")
		{
			std::string name; bool enable;
			af::jr_string("name", name, operation);
			af::jr_bool("enable", enable, operation);
			setService( name, enable);
		}
		else if( type == "restore_defaults")
		{
			m_max_tasks = -1;
			m_capacity = -1;
			m_services_disabled.clear();
			disableServices(); // Dirty check exists in that function
		}
		else
		{
			appendLog("Unknown operation \"" + type + "\" by " + i_action.author);
			return;
		}
		appendLog("Operation \"" + type + "\" by " + i_action.author);
		i_action.monitors->addEvent( af::Msg::TMonitorRendersChanged, m_id);
		AFCommon::QueueDBUpdateItem( this);
		return;
	}

	const JSON & params = (*i_action.data)["params"];
	if( params.IsObject())
		jsonRead( params, &i_action.log);

	if( i_action.log.size() )
	{
		AFCommon::QueueDBUpdateItem( this);
		i_action.monitors->addEvent( af::Msg::TMonitorRendersChanged, m_id);
	}
}

void RenderAf::ejectTasks( JobContainer * jobs, MonitorContainer * monitoring, uint32_t upstatus, const std::string * i_keeptasks_username )
{
	if( m_tasks.size() < 1) return;
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

void RenderAf::exitClient( int type, JobContainer * jobs, MonitorContainer * monitoring)
{
	if( false == isOnline() ) return;
	af::Msg* msg = new af::Msg( type);
	msg->setAddress( this);
	AFCommon::QueueMsgDispatch( msg);
	offline( jobs, af::TaskExec::UPRenderExit, monitoring);
}

void RenderAf::wolSleep( MonitorContainer * monitoring)
{
	//if( isWOLFalling()	) return;
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
	AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
	if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersChanged, m_id);

	af::Msg* msg = new af::Msg( af::Msg::TClientWOLSleepRequest);
	msg->setAddress( this);
	AFCommon::QueueMsgDispatch( msg);
}

void RenderAf::wolWake(  MonitorContainer * i_monitoring, const std::string & i_msg)
{
	if( i_msg.size())
		appendLog( i_msg);

	//if( isWOLWaking()	 ) return;
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
	AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
	if( i_monitoring ) i_monitoring->addEvent( af::Msg::TMonitorRendersChanged, m_id);

	std::string cmd = af::Environment::getRenderCmdWolWake();
	for( int i = 0; i < m_netIFs.size(); i++) cmd += " " + m_netIFs[i]->getMACAddrString( false);

	SysJob::AddWOLCommand( cmd, "", m_name, m_name);
}

void RenderAf::stopTask( int jobid, int blocknum, int tasknum, int number)
{
	if( isOffline()) return;
	af::MCTaskPos taskpos( jobid, blocknum, tasknum, number);
	af::Msg* msg = new af::Msg( af::Msg::TRenderStopTask, &taskpos);
	msg->setAddress( this);
	AFCommon::QueueMsgDispatch( msg);
}

void RenderAf::taskFinished( const af::TaskExec * taskexec, MonitorContainer * monitoring)
{
	removeTask( taskexec);
	remService( taskexec->getServiceType());
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
	if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersChanged, m_id);
}

void RenderAf::addTask( af::TaskExec * taskexec)
{
	// If render was not busy it has become busy now
	if( false == isBusy())
	{
		setBusy( true);
		m_task_start_finish_time = time( NULL);
		AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
	}
	m_tasks.push_back( taskexec);

	m_capacity_used += taskexec->getCapResult();

	if( m_capacity_used > getCapacity() )
		AFERRAR("RenderAf::addTask(): capacity_used > host.capacity (%d>%d)", m_capacity_used, m_host.m_capacity)
}

void RenderAf::removeTask( const af::TaskExec * taskexec)
{
	// Do not set free status here, even if this task was last.
	// May it will take another task in this run cycle

	for( std::list<af::TaskExec*>::iterator it = m_tasks.begin(); it != m_tasks.end(); it++)
	{
		if( *it == taskexec)
		{
			it = m_tasks.erase( it);
			continue;
		}
	}

	if( m_capacity_used < taskexec->getCapResult())
	{
		AFERRAR("RenderAf::removeTask(): capacity_used < taskdata->getCapResult() (%d<%d)", m_capacity_used, taskexec->getCapResult())
		m_capacity_used = 0;
	}
	else m_capacity_used -= taskexec->getCapResult();
}

void RenderAf::v_refresh( time_t currentTime,  AfContainer * pointer, MonitorContainer * monitoring)
{
	if( isLocked() ) return;

	JobContainer * jobs = (JobContainer*)pointer;

	if( isOnline() && (getTimeUpdate() < (currentTime - af::Environment::getRenderZombieTime())))
	{
		appendLog( std::string("ZOMBIETIME: ") + af::itos(af::Environment::getRenderZombieTime()) + " seconds.");
		AFCommon::QueueLog( std::string("Render: \"") + getName() + "\" - ZOMBIETIME");
/*		if( isBusy())
		{
			printf("Was busy:\n");
			for( std::list<af::TaskExec*>::iterator it = tasks.begin(); it != tasks.end(); it++) (*it)->stdOut();
		}*/
		offline( jobs, af::TaskExec::UPRenderZombie, monitoring);
		return;
	}

	// Update busy cpu with no task time:
	if(( isFree() == false ) || isBusy() || ((100 - m_hres.cpu_idle) < m_host.m_busy_cpu ))
		m_busy_time = currentTime;
	else
	{
		// Automatic Nimby ON:
		if(( m_host.m_nimby_busyfree_time > 0 ) && isOnline()
			&& ( currentTime - m_busy_time > m_host.m_nimby_busyfree_time ))
		{
			std::string log("Automatic Nimby: ");
			log += "\n CPU busy since: " + af::time2str( m_busy_time) + " CPU >= " + af::itos( m_host.m_busy_cpu) + "%";
			log += "\n Nimby busy free time = " + af::time2strHMS( m_host.m_nimby_busyfree_time, true );
			appendLog( log);
			setNIMBY();
			monitoring->addEvent( af::Msg::TMonitorRendersChanged, m_id);
			AFCommon::QueueDBUpdateItem( this);
		}
	}
//printf("CPU busy with no task: %li-%li=%li, C%u%%(>%d%%) nbf=%dsecs\n", currentTime, m_busy_time, currentTime-m_busy_time, 100-m_hres.cpu_idle, m_host.m_busy_cpu, m_host.m_nimby_busyfree_time);

	// Update idle time:
	if( isBusy() || ((100 - m_hres.cpu_idle) >= m_host.m_idle_cpu ))
		m_idle_time = currentTime;
	else 
	{
		// Automatic WOL sleep:
		if(( m_host.m_wol_idlesleep_time > 0 ) && isOnline() && ( isWOLSleeping() == false) && ( isWOLFalling() == false)
			&& ( currentTime - m_idle_time > m_host.m_wol_idlesleep_time ))
		{
			std::string log("Automatic WOL Sleep: ");
			log += "\n Idle since: " + af::time2str( m_idle_time) + " CPU < " + af::itos( m_host.m_idle_cpu) + "%";
			log += "\n WOL idle sleep time = " + af::time2strHMS( m_host.m_wol_idlesleep_time, true );
			appendLog( log);
			wolSleep( monitoring);
		}

		// Automatic Nimby Free:
		if(( m_host.m_nimby_idlefree_time > 0 ) && isOnline() && ( isFree() == false)
			&& ( currentTime - m_idle_time > m_host.m_nimby_idlefree_time ))
		{
			std::string log("Automatic Nimby Free: ");
			log += "\n Idle since: " + af::time2str( m_idle_time) + " CPU < " + af::itos( m_host.m_idle_cpu) + "%";
			log += "\n Nimby idle free time = " + af::time2strHMS( m_host.m_nimby_idlefree_time, true );
			appendLog( log);
			setFree();
			monitoring->addEvent( af::Msg::TMonitorRendersChanged, m_id);
			AFCommon::QueueDBUpdateItem( this);
		}
	}
//printf("Idle: %li-%li=%li, C%u%%(<%d%%) (w%d,n%d)secs\n", currentTime, m_idle_time, currentTime-m_idle_time, 100-m_hres.cpu_idle, m_host.m_idle_cpu, m_host.m_wol_idlesleep_time, m_host.m_nimby_idlefree_time);
}

void RenderAf::notSolved()
{
	// If render was busy but has no tasks after solve it is not busy now
	// Needed not to reset busy render status if it run one task after other

	if( isBusy() && ( m_tasks.size() == 0))
	{
		setBusy( false);
		m_task_start_finish_time = time( NULL);
		AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
	}
}

void RenderAf::sendOutput( af::MCListenAddress & mclisten, int JobId, int Block, int Task)
{
	af::Msg * msg = new af::Msg( af::Msg::TTaskListenOutput, &mclisten);
	msg->setAddress( this);
	AFCommon::QueueMsgDispatch( msg);
}

void RenderAf::appendTasksLog( const std::string & message)
{
	while( tasksloglist.size() > af::Environment::getAfNodeLogLinesMax() ) tasksloglist.pop_front();
	tasksloglist.push_back( af::time2str() + " : " + message);
}

bool RenderAf::getFarmHost( af::Host * newHost)
{
	// Store old services usage:
	std::list<int> servicescounts_old;
	std::list<std::string> servicesnames_old;
	for( int i = 0; i < servicesnum; i++)
	{
		servicescounts_old.push_back( servicescounts[i]);
		servicesnames_old.push_back( m_host.getServiceName(i));
	}
	int servicesnum_old = servicesnum;

	// Clear services and services usage:
	m_host.clearServices();
	servicescounts.clear();
	servicesnum = 0;

	// When render becames online it refresh hardware information:
	if( newHost ) m_host.copy( *newHost);

	// Get farm services setttings:
	if( af::farm()->getHost( m_name, m_host, hostname, hostdescription ) == false)
	{
		hostname = "no farm host";
		hostdescription = "";
		return false;
	}

	// Check dirty - check if capacity was overriden and now is equal to the new value
	checkDirty();

	servicesnum = m_host.getServicesNum();
	servicescounts.resize( servicesnum, 0);

	std::list<std::string>::const_iterator osnIt = servicesnames_old.begin();
	std::list<int>::const_iterator oscIt = servicescounts_old.begin();
	for( int o = 0; o < servicesnum_old; o++, osnIt++, oscIt++)
		for( int i = 0; i < servicesnum; i++)
			if( *osnIt == m_host.getServiceName(i))
				servicescounts[i] = *oscIt;

	disableServices();

	return true;
}

void RenderAf::disableServices()
{
//printf("RenderAf::disabledservices: %s\n", m_services_disabled.c_str());
	disabledservices.clear();
	disabledservices.resize( servicesnum, 0);
	if( false == m_services_disabled.empty())
	{
		std::vector<std::string> dissrvlist = af::strSplit( m_services_disabled, ";");
		for( int i = 0; i < dissrvlist.size(); i++)
			for( int j = 0; j < servicesnum; j++)
				if( dissrvlist[i] == m_host.getServiceName(j))
					disabledservices[j] = 1;
	}
	checkDirty();
}

void RenderAf::setService( const std::string & srvname, bool enable)
{
//printf("RenderAf::setService: %s %d\n", srvname.c_str(), enable);
	std::vector<std::string> dissrvlist_old = af::strSplit( m_services_disabled, ";");
	std::vector<std::string> dissrvlist_new;

	// Collect new disabled services list w/o specified service:
	for( int i = 0; i < dissrvlist_old.size(); i++)
		if( dissrvlist_old[i] != srvname )
			dissrvlist_new.push_back( dissrvlist_old[i]);

	// Add specified service in disabled list:
	if( false == enable )
		dissrvlist_new.push_back( srvname);

	if( dissrvlist_new.size())
		m_services_disabled = af::strJoin( dissrvlist_new, ";");
	else
		m_services_disabled.clear();

	disableServices();
}

const std::string RenderAf::getServicesString() const
{
	if( servicesnum == 0) return "No services.";

	std::string str = "Services:";
	for( int i = 0; i < servicesnum; i++)
	{
		str += "\n	";
		str += m_host.getServiceName(i);
		if( disabledservices[i] ) str += " (DISABLED)";
		if(( servicescounts[i] > 0) || ( m_host.getServiceCount(i) > 0))
		{
			str += ": ";
			if( servicescounts[i] > 0) str += af::itos( servicescounts[i]);
			if( m_host.getServiceCount(i) > 0) str += " / max=" + af::itos( m_host.getServiceCount(i));
		}
	}
	if( false == m_services_disabled.empty())
	{
		str += "\nDisabled services:\n	";
		str += m_services_disabled;
	}

	return str;
}
void RenderAf::jsonWriteServices( std::ostringstream & o_str) const
{
	o_str << "\"services\":{";

	for( int i = 0; i < servicesnum; i++)
	{
		if( i > 0 ) o_str << ",";
		o_str << "\"" << m_host.getServiceName(i) << "\":[" << int( servicescounts[i]);
		if( m_host.getServiceCount(i) > 0)
			o_str << ",\"max\"," << int( m_host.getServiceCount(i));
		if( disabledservices[i] ) o_str << ",false";
		o_str << "]";
	}

	o_str << "}";

	if( false == m_services_disabled.empty())
		o_str << ",\"services_disabled\":\"" << m_services_disabled << "\"";

}

bool RenderAf::canRunService( const std::string & type) const
{
	if( false == af::farm()->serviceLimitCheck( type, m_name)) return false;

	for( int i = 0; i < servicesnum; i++)
	{
		if( m_host.getServiceName(i) == type)
		{
			if( disabledservices[i]) return false;
			if( m_host.getServiceCount(i) > 0)
			{
				return servicescounts[i] < m_host.getServiceCount(i);
			}
			return true;
		}
	}
	return false;
}

void RenderAf::addService( const std::string & type)
{
	af::farm()->serviceLimitAdd( type, m_name);

	for( int i = 0; i < servicesnum; i++)
	{
		if( m_host.getServiceName(i) == type)
		{
			servicescounts[i]++;
			if((m_host.getServiceCount(i) > 0 ) && (servicescounts[i] > m_host.getServiceCount(i)))
				AFERRAR("RenderAf::addService: servicescounts > host.getServiceCount for '%s' (%d>=%d)",
						  type.c_str(), servicescounts[i], m_host.getServiceCount(i))
			return;
		}
	}
}

void RenderAf::remService( const std::string & type)
{
	af::farm()->serviceLimitRelease( type, m_name);

	for( int i = 0; i < servicesnum; i++)
	{
		if( m_host.getServiceName(i) == type)
		{
			if( servicescounts[i] < 1)
			{
				AFERRAR("RenderAf::remService: servicescounts < 1 for '%s' (=%d)", type.c_str(), servicescounts[i])
			}
			else
				servicescounts[i]--;
			return;
		}
	}
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

	af::MCTaskPos taskpos( taskup.getNumJob(), taskup.getNumBlock(), taskup.getNumTask(), taskup.getNumber());
	af::Msg* msg = new af::Msg( af::Msg::TRenderCloseTask, &taskpos);
	msg->setAddress( render);
	AFCommon::QueueMsgDispatch( msg);
}

af::Msg * RenderAf::jsonWriteSrvFarm() const
{
	std::ostringstream str;
	str << "{\"object\":{";

	str << "\"name\":\"" << m_name << "\"";
	str << ",\"id\":" << m_id;

	str << ",\"render\":";
	af::Render::v_jsonWrite( str, af::Msg::TRendersList);

	str << ",\"custom_data\":\"" << m_custom_data << '"';

	str << ",";
	jsonWriteServices( str);

	str << ",";
	af::farm()->jsonWriteLimits( str);

	if( isOnline())
	{
		str << ",";
		m_hres.jsonWrite( str);
	}
	
	str << "}}";
	return af::jsonMsg( str);
}

af::Msg * RenderAf::writeFullInfo() const
{
	af::Msg * o_msg = new af::Msg();

	std::string str = v_generateInfoString( true);
	if( m_custom_data.size())
		str += "\nCustom Data:\n" + m_custom_data;
	str += "\n";
	str += getServicesString();
	std::string servicelimits = af::farm()->serviceLimitsInfoString( true);
	if( servicelimits.size())
	{
		str += "\n";
		str += servicelimits;
	}

	o_msg->setString( str);

	return o_msg;
}

int RenderAf::v_calcWeight() const
{
	int weight = Render::v_calcWeight();
//printf("RenderAf::calcWeight: Render::calcWeight: %d bytes\n", weight);
	weight += sizeof(RenderAf) - sizeof( Render);
//printf("RenderAf::calcWeight: %d bytes ( sizeof RenderAf = %d)\n", weight, sizeof( Render));
	return weight;
}
