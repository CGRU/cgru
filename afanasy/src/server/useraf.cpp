#include "useraf.h"

#include <math.h>

#include "../libafanasy/environment.h"

#include "../libafsql/dbattr.h"

#include "afcommon.h"
#include "aflistit.h"
#include "jobaf.h"
#include "renderaf.h"
#include "monitorcontainer.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

UserContainer * UserAf::ms_users = NULL;

UserAf::UserAf( const std::string & username, const std::string & host):
   afsql::DBUser( username, host)
{
   construct();
   appendLog("Registered.");
}

UserAf::UserAf( int uid):
   afsql::DBUser( uid)
{
   construct();
   appendLog("Registered from database.");
}

void UserAf::construct()
{
   m_zombietime = 0;
   m_time_online = time( NULL);
}

UserAf::~UserAf()
{
}

void UserAf::v_priorityChanged( MonitorContainer * i_monitoring) { ms_users->sortPriority( this);}

void UserAf::v_action( const JSON & i_action, const std::string & i_author, std::string & io_changes,
						AfContainer * i_container, MonitorContainer * i_monitoring)
{
	const JSON & params = i_action["params"];
	if( params.IsObject())
		jsonRead( params, &io_changes);

	if( io_changes.size() )
	{
		AFCommon::QueueDBUpdateItem( this);
		if( i_monitoring )
			i_monitoring->addEvent( af::Msg::TMonitorUsersChanged, m_id);
	}
}

bool UserAf::action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring)
{
   std::string userhost( mcgeneral.getUserName() + '@' + mcgeneral.getHostName());
   switch( type)
   {
   case af::Msg::TUserAnnotate:
   {
      m_annotation = mcgeneral.getString();
      appendLog( std::string("Annotation set to \"") + mcgeneral.getString() + "\" by " + userhost);
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_annotation);
      break;
   }
   case af::Msg::TUserHostsMask:
   {
      if( setHostsMask( mcgeneral.getString()))
      {
         appendLog( std::string("Hosts mask set to \"") + mcgeneral.getString() + "\" by " + userhost);
         if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_hostsmask);
      }
      break;
   }
   case af::Msg::TUserHostsMaskExclude:
   {
      if( setHostsMaskExclude( mcgeneral.getString()))
      {
         appendLog( std::string("Exclude hosts mask set to \"") + mcgeneral.getString() + "\" by " + userhost);
         if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_hostsmask_exclude);
      }
      break;
   }
   case af::Msg::TUserMaxRunningTasks:
   {
      m_max_running_tasks = mcgeneral.getNumber();
      appendLog( std::string("Max running tasks set to ") + af::itos( m_max_running_tasks) + " by " + userhost);
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_maxrunningtasks);
      break;
   }
   case af::Msg::TUserPriority:
   {
      m_priority = mcgeneral.getNumber();
      appendLog( std::string("Priority set to ") + af::itos( m_priority) + " by " + userhost);
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_priority);
      break;
   }
   case af::Msg::TUserErrorsAvoidHost:
   {
      m_errors_avoid_host = mcgeneral.getNumber();
      appendLog( std::string("Errors to avoid host set to ") + af::itos( m_errors_avoid_host) + " by " + userhost);
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_errors_avoidhost);
      break;
   }
   case af::Msg::TUserErrorRetries:
   {
      m_errors_retries = mcgeneral.getNumber();
      appendLog( std::string("Errors retries set to ") + af::itos( m_errors_retries) + " by " + userhost);
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_errors_retries);
      break;
   }
   case af::Msg::TUserErrorsTaskSameHost:
   {
      m_errors_task_same_host = mcgeneral.getNumber();
      appendLog( std::string("Errors for task on the same host set to ") + af::itos( m_errors_task_same_host) + " by " + userhost);
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_errors_tasksamehost);
      break;
   }
   case af::Msg::TUserErrorsForgiveTime:
   {
      m_errors_forgive_time = mcgeneral.getNumber();
      appendLog( std::string("Errors forgive time set to ") + af::itos( m_errors_forgive_time) + " by " + userhost);
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_errors_forgivetime);
      break;
   }
   case af::Msg::TUserJobsLifeTime:
   {
      m_jobs_life_time = mcgeneral.getNumber();
      appendLog( std::string("Jobs life time set to ") + af::time2strHMS( m_jobs_life_time, true) + " by " + userhost);
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_lifetime);
      break;
   }
    case af::Msg::TUserJobsSolveMethod:
    {
        setJobsSolveMethod( mcgeneral.getNumber());
        switch( mcgeneral.getNumber())
        {
        case af::Node::SolveByOrder:
         appendLog( std::string("Set jobs solving by order by " + userhost));
         break;
        case af::Node::SolveByPriority:
         appendLog( std::string("Jobs parallel jobs solving by " + userhost));
         break;
        }
        if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
        break;
    }
   case af::Msg::TUserAdd:
   {
      if( false == isPermanent())
      {
         appendLog( std::string("Added by ") + userhost);
         setPermanent( true );
         AFCommon::QueueDBAddItem( this);
      }
      break;
   }
   case af::Msg::TUserDel:
   {
      appendLog( std::string("Deleted by ") + userhost);
      setPermanent( false);
      AFCommon::QueueDBDelItem( this);
      break;
   }
   default:
   {
      return false;
   }
   }
   monitoring->addEvent( af::Msg::TMonitorUsersChanged, m_id);
   return true;
}

void UserAf::setZombie( MonitorContainer * i_monitoring)
{
    AFCommon::QueueLog("Deleting user: " + generateInfoString( false));
    af:Node::setZombie();
    if( i_monitoring ) i_monitoring->addEvent( af::Msg::TMonitorUsersDel, m_id);
    appendLog( "Became a zombie.");
	AFCommon::saveLog( getLog(), af::Environment::getUsersLogsDir(), m_name, af::Environment::getAfNodeLogsRotate());
}

void UserAf::addJob( JobAf * i_job)
{
    appendLog( std::string("Adding a job: ") + i_job->getName());

    m_zombietime = 0;

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
   for( af::Node *job = jobsListIt.node(); job != NULL; jobsListIt.next(), job = jobsListIt.node())
      ((JobAf*)(job))->setUserListOrder( userlistorder++, job != newJob);
}

bool UserAf::getJobs( std::ostringstream & o_str)
{
	AfListIt jobsListIt( &m_jobslist);
	bool first = true;
	bool has_jobs = false;
	for( af::Node *job = jobsListIt.node(); job != NULL; jobsListIt.next(), job = jobsListIt.node())
	{
		if( false == first )
			o_str << ",\n";
		first = false;
		((af::Job*)(job))->v_jsonWrite( o_str, af::Msg::TJobsList);
		has_jobs = true;
	}
	return has_jobs;
}

void UserAf::jobsinfo( af::MCAfNodes &mcjobs)
{
   AfListIt jobsListIt( &m_jobslist);
   for( af::Node *job = jobsListIt.node(); job != NULL; jobsListIt.next(), job = jobsListIt.node())
      mcjobs.addNode( job);
}

void UserAf::refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
/*    if( isLocked() )
    {
        return;
    }*/
//printf("UserAf::refresh: \"%s\"\n", getName().toUtf8().data());
   int _numjobs = m_jobslist.getCount();
   if(( _numjobs == 0) && ( false == isPermanent()))
   {
      if( m_zombietime )
      {
         if( (currentTime-m_zombietime) > af::Environment::getUserZombieTime() )
         {
            appendLog( std::string("ZOMBIETIME: " + af::itos( af::Environment::getUserZombieTime()) + " seconds with no job."));
            setZombie( monitoring);
            return;
         }
      }
      else
      {
         m_zombietime = currentTime;
      }
      return;
   }
   else m_zombietime = 0;

   int _numrunningjobs = 0;
   int _runningtasksnumber = 0;
   {
      AfListIt jobsListIt( &m_jobslist);
      for( af::Node *job = jobsListIt.node(); job != NULL; jobsListIt.next(), job = jobsListIt.node())
      {
         if( ((JobAf*)job)->isRunning())
         {
            _numrunningjobs++;
            _runningtasksnumber += ((JobAf*)job)->getRunningTasksNumber();
         }
      }
   }

   if((( _numrunningjobs      != m_running_jobs_num       ) ||
       ( _numjobs             != m_jobs_num              ) ||
       ( _runningtasksnumber  != m_running_tasks_num   )) &&
         monitoring )
         monitoring->addEvent( af::Msg::TMonitorUsersChanged, m_id);

   m_jobs_num = _numjobs;
   m_running_jobs_num = _numrunningjobs;
   m_running_tasks_num = _runningtasksnumber;

   // Update solving parameters:
   calcNeed();
}

void UserAf::calcNeed()
{
    // Need calculation based on running tasks number
    calcNeedResouces( m_running_tasks_num);
}

bool UserAf::canRun()
{
/*    if( isLocked() )
    {
        return false;
    }*/

    if( m_priority == 0)
    {
        // Zero priority - turns user jobs solving off
        return false;
    }

    if( m_jobs_num < 1 )
    {
        // Nothing to run
        return false;
    }

    // Check maximum running tasks:
    if(( m_max_running_tasks >= 0 ) && ( m_running_tasks_num >= m_max_running_tasks ))
    {
        return false;
    }

    // Returning that node is able run
    return true;
}

bool UserAf::canRunOn( RenderAf * i_render)
{
    if( false == canRun())
    {
        // Unable to run at all
        return false;
    }

// Check nimby:
   if( i_render->isNimby() && (m_name != i_render->getUserName())) return false;

// check hosts mask:
   if( false == checkHostsMask( i_render->getName())) return false;
// check exclude hosts mask:
   if( false == checkHostsMaskExclude( i_render->getName())) return false;

// Returning that user is able to run on specified render
   return true;
}

bool UserAf::solve( RenderAf * i_render, MonitorContainer * i_monitoring)
{
    af::Node::SolvingMethod solve_method = af::Node::SolveByOrder;

    if( solveJobsParallel())
    {
        solve_method = af::Node::SolveByPriority;
    }

    if( m_jobslist.solve( solve_method, i_render, i_monitoring))
    {
        // Increase running tasks counter
        m_running_tasks_num++;

        // Return true - that node was solved
        return true;
    }

    // Return false - that node was not solved
    return false;
}

void UserAf::moveJobs( const af::MCGeneral & mcgeneral, int type)
{
   switch ( type)
   {
      case af::Msg::TUserMoveJobsUp:
      {
         m_jobslist.moveNodes( mcgeneral.getList(), AfList::MoveUp);
         break;
      }
      case af::Msg::TUserMoveJobsDown:
      {
         m_jobslist.moveNodes( mcgeneral.getList(), AfList::MoveDown);
         break;
      }
      case af::Msg::TUserMoveJobsTop:
      {
         m_jobslist.moveNodes( mcgeneral.getList(), AfList::MoveTop);
         break;
      }
      case af::Msg::TUserMoveJobsBottom:
      {
         m_jobslist.moveNodes( mcgeneral.getList(), AfList::MoveBottom);
         break;
      }
      default:
      {
         AFERRAR("UserAf::moveJobs: Invalid type = %d.", type)
         return;
      }
   }
   updateJobsOrder();
}

void UserAf::generateJobsIds( af::MCGeneral & ids) const
{
   if( ids.getCount()) ids.clearIds();
   ids.setId( m_id);
   m_jobslist.generateIds( ids);
}

int UserAf::calcWeight() const
{
   int weight = User::calcWeight();
//printf("UserAf::calcWeight: User::calcWeight: %d bytes\n", weight);
   weight += sizeof(UserAf) - sizeof( User);
//printf("UserAf::calcWeight: %d bytes ( sizeof UserAf = %d)\n", weight, sizeof( UserAf));
   return weight;
}
