#include "useraf.h"

#include <math.h>

#include "../libafanasy/environment.h"

#include "action.h"
#include "afcommon.h"
#include "jobaf.h"
#include "renderaf.h"
#include "solver.h"
#include "monitorcontainer.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

UserContainer * UserAf::ms_users = NULL;

UserAf::UserAf( const std::string & username, const std::string & host):
	af::User( username, host),
	AfNodeSolve( this)
{
	appendLog("Registered from job.");
}

UserAf::UserAf( JSON & i_object):
    af::User(),
	AfNodeSolve( this)
{
	jsonRead( i_object);
}

UserAf::UserAf( const std::string & i_store_dir):
	af::User(),
	AfNodeSolve( this, i_store_dir)
{
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
}

bool UserAf::initialize()
{
	if( isFromStore())
	{
		if(( getTimeRegister() == 0 ) || ( getTimeActivity() == 0 ))
		{
			if( getTimeRegister() == 0 ) setTimeRegister();
			if( getTimeActivity() == 0 ) updateTimeActivity();
			store();
		}
		appendLog("Initialized from store.");
	}
	else
	{
		setTimeRegister();
		updateTimeActivity();
		setStoreDir( AFCommon::getStoreDirUser( *this));
		store();
		appendLog("Registered.");
	}

	return true;
}

UserAf::~UserAf()
{
}

void UserAf::v_action( Action & i_action)
{
	const JSON & operation = (*i_action.data)["operation"];
	if( operation.IsObject())
	{
		std::string type;
		af::jr_string("type", type, operation);
		if( type.find("move_jobs_") == 0 )
		{
			std::vector<int32_t> jids;
			af::jr_int32vec("jids", jids, operation);
			if( type == "move_jobs_up" )
				m_jobslist.moveNodes( jids, AfList::MoveUp);
			else if( type == "move_jobs_down" )
				m_jobslist.moveNodes( jids, AfList::MoveDown);
			else if( type == "move_jobs_top" )
				m_jobslist.moveNodes( jids, AfList::MoveTop);
			else if( type == "move_jobs_bottom" )
				m_jobslist.moveNodes( jids, AfList::MoveBottom);
			updateJobsOrder();
		  	i_action.monitors->addUser( this);
		}
		else if( type == "delete")
		{
			if( m_jobs_num != 0 ) return;
			appendLog( std::string("Deleted by ") + i_action.author);
			deleteNode( i_action.monitors);
			return;
		}
	}

	const JSON & params = (*i_action.data)["params"];
	if( params.IsObject())
		jsonRead( params, &i_action.log);

	if( i_action.log.size() )
	{
		store();
		i_action.monitors->addEvent( af::Monitor::EVT_users_change, m_id);
	}
}

void UserAf::jobPriorityChanged( JobAf * i_job, MonitorContainer * i_monitoring)
{
	AF_DEBUG << "UserAf::jobPriorityChanged:";
	m_jobslist.sortPriority( i_job);
	updateJobsOrder();
	i_monitoring->addUser( this);
}

void UserAf::logAction( const Action & i_action, const std::string & i_node_name)
{
	if( i_action.log.empty())
		return;

	appendLog( std::string("Action[") + i_action.type + "][" +  i_node_name + "]: " + i_action.log);
	updateTimeActivity();
}

void UserAf::deleteNode( MonitorContainer * i_monitoring)
{
	AFCommon::QueueLog("Deleting user: " + v_generateInfoString( false));
	appendLog("Became a zombie.");

	setZombie();

	if( i_monitoring ) i_monitoring->addEvent( af::Monitor::EVT_users_del, m_id);
}

void UserAf::addJob( JobAf * i_job)
{
	appendLog( std::string("Adding a job: ") + i_job->getName());

	updateTimeActivity();

	m_jobslist.add( i_job );

	m_jobs_num++;

	updateJobsOrder( i_job);

	i_job->setUser( this);
}

void UserAf::removeJob( JobAf * i_job)
{
	appendLog( std::string("Removing a job: ") + i_job->getName());

	m_jobslist.remove( i_job );

	m_jobs_num--;
}

void UserAf::updateJobsOrder( af::Job * newJob)
{
	AfListIt jobsListIt( &m_jobslist);
	int userlistorder = 0;
	for( AfNodeSrv *job = jobsListIt.node(); job != NULL; jobsListIt.next(), job = jobsListIt.node())
		((JobAf*)(job))->setUserListOrder( userlistorder++, ((void*)(job)) != ((void*)(newJob)));
}

bool UserAf::getJobs( std::ostringstream & o_str)
{
	AfListIt jobsListIt( &m_jobslist);
	bool first = true;
	bool has_jobs = false;
	for( AfNodeSrv *job = jobsListIt.node(); job != NULL; jobsListIt.next(), job = jobsListIt.node())
	{
		if( false == first )
			o_str << ",\n";
		first = false;
		((JobAf*)(job))->v_jsonWrite( o_str, af::Msg::TJobsList);
		has_jobs = true;
	}
	return has_jobs;
}

void UserAf::jobsinfo( af::MCAfNodes &mcjobs)
{
	AfListIt jobsListIt( &m_jobslist);
	for( AfNodeSrv *job = jobsListIt.node(); job != NULL; jobsListIt.next(), job = jobsListIt.node())
		mcjobs.addNode( job->node());
}

af::Msg * UserAf::writeJobdsOrder( bool i_binary) const
{
	if( i_binary )
	{
		af::MCGeneral ids;
		ids.setId( getId());
		ids.setList( generateJobsIds());
		return new af::Msg( af::Msg::TUserJobsOrder, &ids);
	}


	std::vector<int32_t> jids = m_jobslist.generateIdsList();
	std::ostringstream str;

	str << "{\"events\":{\"jobs_order\":{\"uids\":[";
	str << getId();
	str << "],\"jids\":[[";
	for( int j = 0; j < jids.size(); j++)
	{
		if( j > 0 ) str << ",";
		str << jids[j];
	}
	str << "]]}}}";

	return af::jsonMsg( str);
}

void UserAf::v_refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
	bool changed = refreshCounters();

	if( changed && monitoring )
		monitoring->addEvent( af::Monitor::EVT_users_change, m_id);

	// Update solving parameters:
	v_calcNeed();
}

bool UserAf::refreshCounters()
{
	int _numjobs = m_jobslist.getCount();
	int _numrunningjobs = 0;
	int _runningtasksnumber = 0;
	int _runningcapacitytotal = 0;

	{
		AfListIt jobsListIt( &m_jobslist);
		for( AfNodeSrv *job = jobsListIt.node(); job != NULL; jobsListIt.next(), job = jobsListIt.node())
		{
			if( ((JobAf*)job)->isRunning())
			{
				_numrunningjobs++;
				_runningtasksnumber += ((JobAf*)job)->getRunningTasksNumber();
				_runningcapacitytotal += ((JobAf*)job)->getRunningCapacityTotal();
			}
		}
	}

	bool changed = false;

	if(
		( _numjobs              != m_jobs_num               ) ||
		( _numrunningjobs       != m_running_jobs_num       ) ||
		( _runningtasksnumber   != m_running_tasks_num      ) ||
		( _runningcapacitytotal != m_running_capacity_total ) )
			changed = true;

	m_jobs_num = _numjobs;
	m_running_jobs_num = _numrunningjobs;
	m_running_tasks_num = _runningtasksnumber;
	m_running_capacity_total = _runningcapacitytotal;

	return changed;
}

void UserAf::v_calcNeed()
{
	// Need calculation based on running tasks number
	if( af::Environment::getSolvingUseCapacity())
		calcNeedResouces( m_running_capacity_total);
	else
		calcNeedResouces( m_running_tasks_num);
}

bool UserAf::v_canRun()
{
	if( m_priority == 0 )
	{
		// Zero priority - turns user jobs solving off
		return false;
	}

	if( m_max_running_tasks == 0 )
	{
		// Can't run tasks at all - turns user jobs solving off
		return false;
	}

	if( m_jobs_num < 1 )
	{
		// Nothing to run
		return false;
	}

	// Check maximum running tasks:
	if(( m_max_running_tasks > 0 ) && ( m_running_tasks_num >= m_max_running_tasks ))
	{
		return false;
	}

	// Returning that node is able run
	return true;
}

bool UserAf::v_canRunOn( RenderAf * i_render)
{
// check hosts mask:
	if( false == checkHostsMask( i_render->getName())) return false;
// check exclude hosts mask:
	if( false == checkHostsMaskExclude( i_render->getName())) return false;

// Returning that user is able to run on specified render
	return true;
}

RenderAf * UserAf::v_solve( std::list<RenderAf*> & i_renders_list, MonitorContainer * i_monitoring)
{
	af::Work::SolvingMethod solve_method = af::Work::SolveByOrder;

	if( solveJobsParallel())
	{
		solve_method = af::Work::SolveByPriority;
	}

	std::list<AfNodeSolve*> solve_list( m_jobslist.getStdList());

	RenderAf * render = Solver::SolveList( solve_list, i_renders_list, solve_method);

	if( render )
	{
		// Increase running tasks / total capacity if render is online
		// It can be offline for WOL wake test
		if( render->isOnline())
			refreshCounters();

		// Return solved render
		return render;
	}

	// Node was not solved
	return NULL;
}

int UserAf::v_calcWeight() const
{
	int weight = User::v_calcWeight();
//printf("UserAf::calcWeight: User::calcWeight: %d bytes\n", weight);
	weight += sizeof(UserAf) - sizeof( User);
//printf("UserAf::calcWeight: %d bytes ( sizeof UserAf = %d)\n", weight, sizeof( UserAf));
	return weight;
}
