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
	This is a server side of an Afanasy user.
*/
#include "useraf.h"

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
				m_jobs_list.moveNodes( jids, AfList::MoveUp);
			else if( type == "move_jobs_down" )
				m_jobs_list.moveNodes( jids, AfList::MoveDown);
			else if( type == "move_jobs_top" )
				m_jobs_list.moveNodes( jids, AfList::MoveTop);
			else if( type == "move_jobs_bottom" )
				m_jobs_list.moveNodes( jids, AfList::MoveBottom);
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
	m_jobs_list.sortPriority( i_job);
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

	// Add running counts (runnig tasks num and capacity total) to Af::Work
	addRunningCounts(*i_job);
	// Add renders counts (for max tasks per host) to AfNodeSolve
	addRendersCounts(*i_job);

	m_jobs_list.add( i_job );

	m_jobs_num++;

	updateJobsOrder( i_job);

	i_job->setUser( this);
}

void UserAf::removeJob( JobAf * i_job)
{
	appendLog( std::string("Removing a job: ") + i_job->getName());

	// Remove running counts (runnig tasks num and capacity total) from Af::Work
	remRunningCounts(*i_job);
	// Remove renders counts (for max tasks per host) from AfNodeSolve
	remRendersCounts(*i_job);

	m_jobs_list.remove( i_job );

	m_jobs_num--;
}

void UserAf::updateJobsOrder( af::Job * newJob)
{
	AfListIt jobsListIt( &m_jobs_list);
	int userlistorder = 0;
	for( AfNodeSrv *job = jobsListIt.node(); job != NULL; jobsListIt.next(), job = jobsListIt.node())
		((JobAf*)(job))->setUserListOrder( userlistorder++, ((void*)(job)) != ((void*)(newJob)));
}

bool UserAf::getJobs( std::ostringstream & o_str)
{
	AfListIt jobsListIt( &m_jobs_list);
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
	AfListIt jobsListIt( &m_jobs_list);
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


	std::vector<int32_t> jids = m_jobs_list.generateIdsList();
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
}

bool UserAf::refreshCounters()
{
	int _numjobs = m_jobs_list.getCount();
	int _numrunningjobs = 0;

	AfListIt jobsListIt(&m_jobs_list);
	for (AfNodeSrv *job = jobsListIt.node(); job != NULL; jobsListIt.next(), job = jobsListIt.node())
		if (((JobAf*)job)->isRunning())
			_numrunningjobs++;

	bool changed = false;

	if (( _numjobs              != m_jobs_num               ) ||
		( _numrunningjobs       != m_running_jobs_num       ))
			changed = true;

	m_jobs_num = _numjobs;
	m_running_jobs_num = _numrunningjobs;

	return changed;
}

bool UserAf::v_canRun()
{
	if( m_jobs_num < 1 )
	{
		// Nothing to run
		return false;
	}

	// Returning that node is able run
	return true;
}

bool UserAf::v_canRunOn( RenderAf * i_render)
{
	// Returning that user is able to run on specified render
	return true;
	//^No more checks above AfNodeSolve::canRunOn() needed
}
#include "branchsrv.h"
RenderAf * UserAf::v_solve( std::list<RenderAf*> & i_renders_list, MonitorContainer * i_monitoring, BranchSrv * i_branch)
{
	std::list<AfNodeSolve*> solve_list;

	if (NULL == i_branch)
	{
		AF_ERR << "UserAf::v_solve: '" << getName() << "' i_branch is NULL.";
		return NULL;
	}

	AfListIt it(&m_jobs_list);
	for (AfNodeSolve * node = it.node(); node != NULL; it.next(), node = it.node())
	{
		if (i_branch != (static_cast<JobAf*>(node))->getBranchPtr())
			continue;

		if (false == node->canRun())
			continue;

		node->calcNeed(m_solving_flags);

		solve_list.push_back(node);
	}

	if (isSolvePriority())
		Solver::SortList(solve_list, m_solving_flags);

	return Solver::SolveList(solve_list, i_renders_list, NULL);
}

void UserAf::addSolveCounts(MonitorContainer * i_monitoring, af::TaskExec * i_exec, RenderAf * i_render)
{
	AfNodeSolve::addSolveCounts(i_exec, i_render);
	i_monitoring->addEvent(af::Monitor::EVT_users_change, getId());
}

void UserAf::remSolveCounts(MonitorContainer * i_monitoring, af::TaskExec * i_exec, RenderAf * i_render)
{
	AfNodeSolve::remSolveCounts(i_exec, i_render);
	i_monitoring->addEvent(af::Monitor::EVT_users_change, getId());
}

int UserAf::v_calcWeight() const
{
	int weight = User::v_calcWeight();
	weight += sizeof(UserAf) - sizeof( User);
	return weight;
}
