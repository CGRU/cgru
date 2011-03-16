#include "useraf.h"

#include <math.h>

#include "../libafanasy/environment.h"

#include "../libafsql/dbattr.h"

#include "afcommon.h"
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
      appendLog( std::string("Jobs life time set to ") + af::itos( jobs_lifetime) + " seconds by %2" + userhost);
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_lifetime);
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
   int userlistorder = jobs.addJob( job );
   numjobs++;
   updateJobsOrder( job);
   return userlistorder;
}

void UserAf::updateJobsOrder( af::Job * newJob)
{
   JobsListIt jobsListIt( &jobs);
   int userlistorder = 0;
   for( af::Job *job = jobsListIt.job(); job != NULL; jobsListIt.next(), job = jobsListIt.job())
      ((JobAf*)(job))->setUserListOrder( userlistorder++, job != newJob);
}

void UserAf::jobsinfo( af::MCAfNodes &mcjobs)
{
   JobsListIt jobsListIt( &jobs);
   for( af::Job *job = jobsListIt.job(); job != NULL; jobsListIt.next(), job = jobsListIt.job())
      mcjobs.addNode( job);
}

void UserAf::refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
   if( isLocked() ) return;
//printf("UserAf::refresh: \"%s\"\n", getName().toUtf8().data());
   int _numjobs = jobs.getCount();
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
      JobsListIt jobsListIt( &jobs);
      for( af::Job *job = jobsListIt.job(); job != NULL; jobsListIt.next(), job = jobsListIt.job())
      {
         if( job->isRunning())
         {
            _numrunningjobs++;
            _runningtasksnumber += job->getRunningTasksNumber();
         }
      }
   }

   float _need = need;
   calcNeed();

   if((( _numrunningjobs      != numrunningjobs       ) ||
       ( _numjobs             != numjobs              ) ||
       ( _runningtasksnumber  != runningtasksnumber   ) ||
       ( _need                != need                 )) &&
         monitoring )
         monitoring->addEvent( af::Msg::TMonitorUsersChanged, id);

   numjobs = _numjobs;
   numrunningjobs = _numrunningjobs;
   runningtasksnumber = _runningtasksnumber;
}

bool UserAf::canRun( RenderAf *render)
{
   if( priority == 0) return false;

   if( need == 0.0) return false;

   if( render->isNimby() && (name != render->getUserName())) return false;

// check maximum hosts:
   if(( maxrunningtasks >= 0 ) && ( runningtasksnumber >= maxrunningtasks )) return false;

// check hosts mask:
   if( false == checkHostsMask( render->getName())) return false;
// check exclude hosts mask:
   if( false == checkHostsMaskExclude( render->getName())) return false;

   return true;
}

bool UserAf::genTask( RenderAf *render, MonitorContainer * monitoring)
{
// search for ready job:
   JobsListIt jobsListIt( &jobs);
   for( JobAf *job = jobsListIt.job(); job != NULL; jobsListIt.next(), job = jobsListIt.job())
   {
      if( job->solve( render, monitoring))
      {
         runningtasksnumber++;
         return true;
      }
   }
   return false;
}

void UserAf::moveJobs( const af::MCGeneral & mcgeneral, int type)
{
   switch ( type)
   {
      case af::Msg::TUserMoveJobsUp:
      {
         jobs.moveNodes( mcgeneral.getList(), AfList::MoveUp);
         break;
      }
      case af::Msg::TUserMoveJobsDown:
      {
         jobs.moveNodes( mcgeneral.getList(), AfList::MoveDown);
         break;
      }
      case af::Msg::TUserMoveJobsTop:
      {
         jobs.moveNodes( mcgeneral.getList(), AfList::MoveTop);
         break;
      }
      case af::Msg::TUserMoveJobsBottom:
      {
         jobs.moveNodes( mcgeneral.getList(), AfList::MoveBottom);
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
   jobs.generateIds( ids);
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
   if( priority == 0)
   {
      need = 0;
      return;
   }
   if( numjobs == 0)
   {
      need = 0;
      return;
   }
   need = pow( 1.1, priority) / (runningtasksnumber + 1.0);
}
