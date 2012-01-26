#include "useraf.h"

#include <math.h>

#include "../libafanasy/environment.h"

#include "../libafsql/dbattr.h"

#include "afcommon.h"
#include "aflistit.h"
#include "jobaf.h"
#include "renderaf.h"
#include "monitorcontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

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
   zombietime = 0;
   time_online = time( NULL);
}

UserAf::~UserAf()
{
}

bool UserAf::action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring)
{
   std::string userhost( mcgeneral.getUserName() + '@' + mcgeneral.getHostName());
   switch( type)
   {
   case af::Msg::TUserAnnotate:
   {
      annotation = mcgeneral.getString();
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
      maxrunningtasks = mcgeneral.getNumber();
      appendLog( std::string("Max running tasks set to ") + af::itos( maxrunningtasks) + " by " + userhost);
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_maxrunningtasks);
      break;
   }
   case af::Msg::TUserPriority:
   {
      priority = mcgeneral.getNumber();
      appendLog( std::string("Priority set to ") + af::itos( priority) + " by " + userhost);
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_priority);
      break;
   }
   case af::Msg::TUserErrorsAvoidHost:
   {
      errors_avoidhost = mcgeneral.getNumber();
      appendLog( std::string("Errors to avoid host set to ") + af::itos( errors_avoidhost) + " by " + userhost);
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_errors_avoidhost);
      break;
   }
   case af::Msg::TUserErrorRetries:
   {
      errors_retries = mcgeneral.getNumber();
      appendLog( std::string("Errors retries set to ") + af::itos( errors_retries) + " by " + userhost);
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_errors_retries);
      break;
   }
   case af::Msg::TUserErrorsTaskSameHost:
   {
      errors_tasksamehost = mcgeneral.getNumber();
      appendLog( std::string("Errors for task on the same host set to ") + af::itos( errors_tasksamehost) + " by " + userhost);
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_errors_tasksamehost);
      break;
   }
   case af::Msg::TUserErrorsForgiveTime:
   {
      errors_forgivetime = mcgeneral.getNumber();
      appendLog( std::string("Errors forgive time set to ") + af::itos( errors_forgivetime) + " by " + userhost);
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_errors_forgivetime);
      break;
   }
   case af::Msg::TUserJobsLifeTime:
   {
      jobs_lifetime = mcgeneral.getNumber();
      appendLog( std::string("Jobs life time set to ") + af::time2strHMS( jobs_lifetime, true) + " by " + userhost);
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
   monitoring->addEvent( af::Msg::TMonitorUsersChanged, id);
   return true;
}

void UserAf::setZombie()
{
   AFCommon::QueueLog("Deleting user: " + generateInfoString( false));
   af::Node::setZombie();
   appendLog( "Became a zombie.");
   AFCommon::saveLog( log, af::Environment::getUsersLogsDir(), name, af::Environment::getUserLogsRotate());
}

int UserAf::addJob( JobAf *job)
{
   appendLog( std::string("Adding a job: ") + job->getName());
   zombietime = 0;
   int userlistorder = m_jobslist.add( job );
   numjobs++;
   updateJobsOrder( job);
   return userlistorder;
}

void UserAf::updateJobsOrder( af::Job * newJob)
{
   AfListIt jobsListIt( &m_jobslist);
   int userlistorder = 0;
   for( af::Node *job = jobsListIt.node(); job != NULL; jobsListIt.next(), job = jobsListIt.node())
      ((JobAf*)(job))->setUserListOrder( userlistorder++, job != newJob);
}

void UserAf::jobsinfo( af::MCAfNodes &mcjobs)
{
   AfListIt jobsListIt( &m_jobslist);
   for( af::Node *job = jobsListIt.node(); job != NULL; jobsListIt.next(), job = jobsListIt.node())
      mcjobs.addNode( job);
}

void UserAf::refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
   if( isLocked() ) return;
//printf("UserAf::refresh: \"%s\"\n", getName().toUtf8().data());
   int _numjobs = m_jobslist.getCount();
   if(( _numjobs == 0) && ( false == isPermanent()))
   {
      if( zombietime )
      {
         if( (currentTime-zombietime) > af::Environment::getUserZombieTime() )
         {
            appendLog( std::string("ZOMBIETIME: " + af::itos( af::Environment::getUserZombieTime()) + " seconds with no job."));
            setZombie();
            if( monitoring ) monitoring->addEvent( af::Msg::TMonitorUsersDel, id);
            return;
         }
      }
      else
      {
         zombietime = currentTime;
      }
      return;
   }
   else zombietime = 0;

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

   if((( _numrunningjobs      != numrunningjobs       ) ||
       ( _numjobs             != numjobs              ) ||
       ( _runningtasksnumber  != runningtasksnumber   )) &&
         monitoring )
         monitoring->addEvent( af::Msg::TMonitorUsersChanged, id);

   numjobs = _numjobs;
   numrunningjobs = _numrunningjobs;
   runningtasksnumber = _runningtasksnumber;

   calcNeed();
}

bool UserAf::canRun()
{
    if( priority == 0)
    {
        // Zero priority - turns user jobs solving off
        return false;
    }

    if( numjobs < 1 )
    {
        // Nothing to run
        return false;
    }

    // Check maximum running tasks:
    if(( maxrunningtasks >= 0 ) && ( runningtasksnumber >= maxrunningtasks ))
    {
        return false;
    }

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
   if( i_render->isNimby() && (name != i_render->getUserName())) return false;

// check hosts mask:
   if( false == checkHostsMask( i_render->getName())) return false;
// check exclude hosts mask:
   if( false == checkHostsMaskExclude( i_render->getName())) return false;

   return true;
}

bool UserAf::solve( RenderAf * i_render, MonitorContainer * i_monitoring)
{
    af::Node::SolvingMethod solve_method = af::Node::SolveByOrder;

    if( solveJobsParrallel())
    {
        solve_method = af::Node::SolveByPriority;
    }

    if( m_jobslist.solve( solve_method, i_render, i_monitoring))
    {
        // Increase running tasks counter;
        runningtasksnumber++;

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

void UserAf::appendLog( const std::string & message)
{
   log.push_back( af::time2str() + " : " + message);
   while( log.size() > af::Environment::getUserLogLinesMax() ) log.pop_front();
}

void UserAf::generateJobsIds( af::MCGeneral & ids) const
{
   if( ids.getCount()) ids.clearIds();
   ids.setId( id);
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

void UserAf::calcNeed()
{
    // Need calculation based on running tasks number
    calcNeedResouces( runningtasksnumber);
}
