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

UserAf::UserAf( const QString &username, const QString &host):
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
   QString userhost( mcgeneral.getUserName()+'@'+mcgeneral.getHostName());
   switch( type)
   {
   case af::Msg::TUserHostsMask:
   {
      if( setHostsMask( mcgeneral.getString()))
      {
         appendLog( QString("Hosts mask set to \"%1\" by %2").arg(mcgeneral.getString(), userhost));
         if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_hostsmask);
      }
      break;
   }
   case af::Msg::TUserHostsMaskExclude:
   {
      if( setHostsMaskExclude( mcgeneral.getString()))
      {
         appendLog( QString("Hosts mask set to \"%1\" by %2").arg(mcgeneral.getString(), userhost));
         if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_hostsmask_exclude);
      }
      break;
   }
   case af::Msg::TUserMaxHosts:
   {
      appendLog( QString("Maximum hosts set to %1 by %2").arg(mcgeneral.getNumber()).arg(userhost));
      maxhosts = mcgeneral.getNumber();
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_maxhosts);
      break;
   }
   case af::Msg::TUserPriority:
   {
      appendLog( QString("Priority set to %1 by %2").arg(mcgeneral.getNumber()).arg(userhost));
      priority = mcgeneral.getNumber();
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_priority);
      break;
   }
   case af::Msg::TUserErrorsAvoidHost:
   {
      appendLog( QString("Errors to avoid host set to %1 by %2").arg(mcgeneral.getNumber()).arg(userhost));
      errors_avoidhost = mcgeneral.getNumber();
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_errors_avoidhost);
      break;
   }
   case af::Msg::TUserErrorRetries:
   {
      appendLog( QString("Errors retries set to %1 by %2").arg(mcgeneral.getNumber()).arg(userhost));
      errors_retries = mcgeneral.getNumber();
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_errors_retries);
      break;
   }
   case af::Msg::TUserErrorsTaskSameHost:
   {
      appendLog( QString("Errors for task on same host set to %1 by %2").arg(mcgeneral.getNumber()).arg(userhost));
      errors_tasksamehost = mcgeneral.getNumber();
      if( isPermanent()) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_errors_tasksamehost);
      break;
   }
   case af::Msg::TUserAdd:
   {
      appendLog( QString("Added by %1").arg(userhost));
      setPermanent( true );
      AFCommon::QueueDBAddItem( this);
      break;
   }
   case af::Msg::TUserDel:
   {
      appendLog( QString("Deleted by %1").arg(userhost));
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
   af::printTime(); printf(" : Deleting user: "); stdOut( false);
   af::Node::setZombie();
   appendLog( "Became a zombie.");
   AFCommon::saveLog( log, af::Environment::getUsersLogsDir(), name, af::Environment::getUserLogsRotate());
}

void UserAf::addJob( JobAf *job)
{
   appendLog( QString("Adding a job: %1").arg(job->getName()));
   zombietime = 0;
   jobs.addJob( job );
   numjobs++;
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
            appendLog( QString("ZOMBIETIME: %1 seconds with no job.").arg( af::Environment::getUserZombieTime()));
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
   int _numhosts = 0;
   {
      JobsListIt jobsListIt( &jobs);
      for( af::Job *job = jobsListIt.job(); job != NULL; jobsListIt.next(), job = jobsListIt.job())
      {
         if( job->isRunning())
         {
            _numrunningjobs++;
            _numhosts += job->getNumRunningTasks();
         }
      }
   }

   float _need = need;
   calcNeed();

   if((( _numrunningjobs != numrunningjobs) ||
       ( _numjobs != numjobs) ||
       ( _numhosts != numhosts) ||
       ( _need != need)) &&
      monitoring )
         monitoring->addEvent( af::Msg::TMonitorUsersChanged, id);

   numjobs = _numjobs;
   numrunningjobs = _numrunningjobs;
   numhosts = _numhosts;
}

bool UserAf::canRun( RenderAf *render)
{
   if( priority == 0) return false;

   if( need == 0.0) return false;

   if( render->isNimby() && (name != render->getUserName())) return false;

// check maximum hosts:
   if(( maxhosts >= 0 ) && ( numhosts >= maxhosts )) return false;
// check hosts mask:
   if( false == checkHostsMask( render->getName())) return false;
// check exclude hosts mask:
   if( true == checkHostsMaskExclude( render->getName())) return false;

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
         numhosts++;
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
         AFERRAR("UserAf::moveJobs: Invalid type = %d.\n", type);
         break;
      }
   }
}

void UserAf::appendLog( const QString &message)
{
   while( log.size() > af::Environment::getUserLogsLinesMax() ) log.removeFirst();
   log << af::time2Qstr() + " : " + message;
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
   need = pow( 1.1, priority) / (numhosts + 1.0);
}
