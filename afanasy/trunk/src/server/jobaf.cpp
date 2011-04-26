#include "jobaf.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/blockdata.h"
#include "../libafanasy/addresseslist.h"
#include "../libafanasy/msgclasses/mclistenaddress.h"

#include "../libafsql/dbattr.h"
#include "../libafsql/qdbconnection.h"
#include "../libafsql/dbjobprogress.h"

#include "afcommon.h"
#include "block.h"
#include "jobcontainer.h"
#include "monitorcontainer.h"
#include "msgqueue.h"
#include "renderaf.h"
#include "rendercontainer.h"
#include "sysjob.h"
#include "task.h"
#include "useraf.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

JobContainer *JobAf::jobs  = NULL;

JobAf::JobAf( af::Msg * msg):
   afsql::DBJob(),
   fromdatabase( false)
{
   initializeValues();
   read( msg);
   progress = new afsql::DBJobProgress( this);
   if( progress == NULL)
   {
      AFERROR("DBJob::DBJob: can't allocate memory for progresses.")
      return;
   }
   construct();
}

JobAf::JobAf( int Id):
   afsql::DBJob( Id),
   fromdatabase( true)
{
   initializeValues();
}

void JobAf::initializeValues()
{
   blocks            = NULL;
   constructed       = false;
   initialized       = false;
   deletion          = false;
   logsWeight        = 0;
   blackListsWeight  = 0;
}

bool JobAf::dbSelect( QSqlDatabase * db, const QString * where)
{
//printf("JobAf::dbSelect:\n");
   if( afsql::DBJob::dbSelect( db) == false) return false;
   return construct();
}

bool JobAf::construct()
{
//printf("JobAf::construct:\n");
   if( constructed )
   {
      AFERROR("JobAf::construct: Already constructed.")
      return true;
   }

   if( blocksnum < 1 )
   {
      AFERRAR("JobAf::construct: invalid number of blocks = %d", blocksnum)
      return false;
   }

   blocks = new Block*[blocksnum];
   if( blocks == NULL )
   {
      AFERROR("JobAf::construct: Can't allocate memory for blocks.")
      return false;
   }
   for( int b = 0; b < blocksnum; b++) blocks[b] = NULL;
   for( int b = 0; b < blocksnum; b++)
   {
      if( blocksdata[b]->isValid() == false) return false;
      blocks[b] = newBlock(b);
      if( blocks[b] == NULL )
      {
         AFERRAR("JobAf::construct: Can't create block %d of %d.", b, blocksnum)
         return false;
      }
   }

   constructed = true;
   return true;
}

JobAf::~JobAf()
{
   if( blocks )
   {
      for( int b = 0; b < blocksnum; b++) if( blocks[b]) delete blocks[b];
      delete [] blocks;
   }
}

Block * JobAf::newBlock( int numBlock)
{
   return new Block( this, blocksdata[numBlock], progress, &joblog);
}

bool JobAf::initialize( UserAf * jobOwner)
{
   if( isConstructed() == false)
   {
      AFERROR("JobAf::initialize: Job is not constructed.")
      return 0;
   }

//printf("JobAf::initialize: BEGIN\n");
   user = jobOwner;
   progress->setJobId( id);
   for( int b = 0; b < blocksnum; b++)
   {
      blocksdata[b]->setJobId( id);
      blocks[b]->setUser( jobOwner);
   }

//
// Create tasks output directory ( if needed )
   tasksoutputdir = name;
   af::pathFilterFileName( tasksoutputdir);
   tasksoutputdir = af::Environment::getTasksStdOutDir() + '/' + tasksoutputdir;
   if( af::pathMakeDir( tasksoutputdir) == false)
   {
      log( std::string("Unable to create tasks output directory:\n") + tasksoutputdir);
      return false;
   }

//
// Executing pre commands ( if not from database )

   if( fromdatabase == false )
   {
      if( false == cmd_pre.empty())
      {
         AFCommon::executeCmd( cmd_pre);
         log( std::string("Job pre command executed:\n") + cmd_pre);
      }
      for( int b = 0; b < blocksnum; b++)
      {
         if( blocksdata[b]->hasCmdPre() )
         {
            AFCommon::executeCmd( blocksdata[b]->getCmdPre());
            log( std::string("Block[") + blocksdata[b]->getName() + "] pre command executed:\n" + blocksdata[b]->getCmdPre());
         }
      }
      log("Initialized.");
   }
   else
   {
      log("Initialized from database.");
   }

//
// Checking states

   for( int b = 0; b < blocksnum; b++)
   {
//      if( b == 0 ) blocksdata[b]->state = blocksdata[b]->state | AFJOB::STATE_READY_MASK;
//      blocksdata[b]->state = blocksdata[b]->state & (~AFJOB::STATE_WAITDEP_MASK);
      int numtasks = blocksdata[b]->getTasksNum();
      for( int t = 0; t < numtasks; t++)
      {
         uint32_t taskstate = progress->tp[b][t]->state;
         if((  taskstate == 0                           ) ||
            (  taskstate & AFJOB::STATE_RUNNING_MASK   ))
         {
            taskstate = taskstate |   AFJOB::STATE_READY_MASK;
            taskstate = taskstate & (~AFJOB::STATE_RUNNING_MASK);
            progress->tp[b][t]->state = taskstate;
         }
      }
   }

   if(( state & AFJOB::STATE_DONE_MASK) == false ) state = state | AFJOB::STATE_WAITDEP_MASK;

   refresh( time(NULL), NULL, NULL);

   initialized = true;

//printf("JobAf::initialize: END\n");
   return true;
}

int JobAf::getUid() const { return user->getId(); }

void JobAf::setZombie( RenderContainer * renders, MonitorContainer * monitoring)
{
   if( deletion == false )
   {
      state = AFJOB::STATE_OFFLINE_MASK;
      lock();
      deletion = true;
      if( getRunningTasksNumber() && (renders != NULL) && (monitoring != NULL))
      {
//printf("JobAf::setZombie: runningtaskscounter = %d\n", runningtaskscounter);
         restartAllTasks( true, "Job deletion.", renders, monitoring);
         if( monitoring ) monitoring->addJobEvent( af::Msg::TMonitorJobsChanged, getId(), getUid());
         return;
      }
   }
   if( getRunningTasksNumber() )
   {
      AFERRAR("JobAf::setZombie: runningtaskscounter = %d", getRunningTasksNumber())
      return;
   }

   if( false == cmd_post.empty())
   {
      SysJob::AddPostCommand( cmd_post, blocksnum > 0 ? blocksdata[0]->getWDir(): "", username, name);
      log( std::string("Executing job post command:\n") + cmd_post);
   }
   for( int b = 0; b < blocksnum; b++)
   {
      if( blocksdata[b]->hasCmdPost())
      {
         SysJob::AddPostCommand( blocksdata[b]->getCmdPost(), blocksdata[b]->getWDir(), username, name);
         log( std::string("Executing block[") + blocksdata[b]->getName() + "] post command:\n" + blocksdata[b]->getCmdPost());
      }
   }
   Node::setZombie();

   // Queue job cleanup:
   AFCommon::QueueJobCleanUp( this);

   if( isInitialized()) AFCommon::QueueDBDelItem( this);
   if( monitoring ) monitoring->addJobEvent( af::Msg::TMonitorJobsDel, getId(), getUid());
   AFCommon::QueueLog("Deleting a job: " + generateInfoString());
   unLock();
}

bool JobAf::action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring)
{
//printf("JobAf::action: %s\n", af::Msg::TNAMES[type]);mcgeneral.stdOut();
   uint32_t jobchanged = 0;
   std::string userhost = mcgeneral.getUserName() + '@' + mcgeneral.getHostName();
   switch( type)
   {
   case af::Msg::TJobAnnotate:
   {
      annotation = mcgeneral.getString();
      log( std::string("Annotation set to \"") + mcgeneral.getString() + "\" by " + userhost);
      jobchanged = af::Msg::TMonitorJobsChanged;
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_annotation);
      break;
   }
   case af::Msg::TJobHostsMask:
   {
      if( setHostsMask( mcgeneral.getString()))
      {
         log( std::string("Hosts mask set to \"") + mcgeneral.getString() + "\" by " + userhost);
         jobchanged = af::Msg::TMonitorJobsChanged;
         AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_hostsmask);
      }
      break;
   }
   case af::Msg::TJobHostsMaskExclude:
   {
      if( setHostsMaskExclude( mcgeneral.getString()))
      {
         log( std::string("Exclude hosts mask set to \"") + mcgeneral.getString() + "\" by " + userhost);
         jobchanged = af::Msg::TMonitorJobsChanged;
         AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_hostsmask_exclude);
      }
      break;
   }
   case af::Msg::TJobDependMask:
   {
      if( setDependMask( mcgeneral.getString()))
      {
         log( std::string("Depend mask set to \"") + mcgeneral.getString() + "\" by " + userhost);
         jobchanged = af::Msg::TMonitorJobsChanged;
         AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_dependmask);
      }
      break;
   }
   case af::Msg::TJobDependMaskGlobal:
   {
      if( setDependMaskGlobal( mcgeneral.getString()))
      {
         log( std::string("Global depend mask set to \"") + mcgeneral.getString() + "\" by " + userhost);
         jobchanged = af::Msg::TMonitorJobsChanged;
         AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_dependmask_global);
      }
      break;
   }
   case af::Msg::TJobMaxRunningTasks:
   {
      maxrunningtasks = mcgeneral.getNumber();
      log( std::string("Max running tasks set to ") + af::itos( maxrunningtasks) + " by " + userhost);
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_maxrunningtasks);
      jobchanged = af::Msg::TMonitorJobsChanged;
      break;
   }
   case af::Msg::TJobWaitTime:
   {
      time_wait = mcgeneral.getNumber();
      log( std::string("Wait time set to \"") + af::time2str( time_wait) + "\" by " + userhost);
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_time_wait);
      jobchanged = af::Msg::TMonitorJobsChanged;
      break;
   }
   case af::Msg::TJobLifeTime:
   {
      lifetime = mcgeneral.getNumber();
      log( std::string("Life time set to ") + af::time2strHMS( lifetime, true) + " by " + userhost);
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_lifetime);
      jobchanged = af::Msg::TMonitorJobsChanged;
      break;
   }
   case af::Msg::TJobPriority:
   {
      priority = mcgeneral.getNumber();
      log( std::string("Priority set to ") + af::itos( priority) + " by " + userhost);
      jobchanged = af::Msg::TMonitorJobsChanged;
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_priority);
      if( monitoring ) monitoring->addUser( user);
      break;
   }
   case af::Msg::TJobNeedOS:
   {
      if( setNeedOS( mcgeneral.getString()))
      {
         log( std::string("Needed OS set to \"") + mcgeneral.getString() + "\" by " + userhost);
         jobchanged = af::Msg::TMonitorJobsChanged;
         AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_need_os);
      }
      break;
   }
   case af::Msg::TJobNeedProperties:
   {
      if( setNeedProperties( mcgeneral.getString()))
      {
         log( std::string("Needed properties set to \"") + mcgeneral.getString() + "\" by " + userhost);
         jobchanged = af::Msg::TMonitorJobsChanged;
         AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_need_properties);
      }
      break;
   }
   case af::Msg::TJobCmdPost:
   {
      cmd_post = mcgeneral.getString();
      log( std::string("Post command set to \"") + cmd_post + "\" by " + userhost);
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_cmd_post);
      jobchanged = af::Msg::TMonitorJobsChanged;
      break;
   }
   case af::Msg::TJobStart:
   {
      state = state & (~AFJOB::STATE_OFFLINE_MASK);
      log("Started by " + userhost);
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
      jobchanged = af::Msg::TMonitorJobsChanged;
      break;
   }
   case af::Msg::TJobStop:
   {
      log("Stopped by " + userhost);
      restartAllTasks( true, "Job stopped by " + userhost, (RenderContainer*)pointer, monitoring);
      state = state | AFJOB::STATE_OFFLINE_MASK;
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
      jobchanged = af::Msg::TMonitorJobsChanged;
      break;
   }
   case af::Msg::TJobRestart:
   {
      log(            "Restarted by " + userhost );
//printf("Msg::TJobRestart:\n");
      restartAllTasks( false, "Job restarted by " + userhost, (RenderContainer*)pointer, monitoring);
//printf("Msg::TJobRestart: tasks restarted.\n");
      checkDepends();
      time_started = 0;
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_time_started);
      jobchanged = af::Msg::TMonitorJobsChanged;
      break;
   }
   case af::Msg::TJobRestartErrors:
   {
      log(      "Errors restarted by " + userhost );
      restartErrors( "Job errors restarted by " + userhost, (RenderContainer*)pointer, monitoring);
      jobchanged = af::Msg::TMonitorJobsChanged;
      break;
   }
   case af::Msg::TJobPause:
   {
      log( "Paused by " + userhost );
      state = state | AFJOB::STATE_OFFLINE_MASK;
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
      jobchanged = af::Msg::TMonitorJobsChanged;
      break;
   }
   case af::Msg::TJobRestartPause:
   {
      log( "Restarted ( and paused ) by " + userhost );
      restartAllTasks( false, "Job restarted ( and paused ) by " + userhost, (RenderContainer*)pointer, monitoring);
      checkDepends();
      state = state | AFJOB::STATE_OFFLINE_MASK;
      time_started = 0;
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_time_started);
      jobchanged = af::Msg::TMonitorJobsChanged;
      break;
   }
   case af::Msg::TJobDelete:
   {
      log( "Deleted by " + userhost);
      user->appendLog( std::string("Job \"") + name + "\" deleted by " + userhost);
      setZombie( (RenderContainer*)pointer, monitoring);
      if( monitoring ) monitoring->addJobEvent( af::Msg::TMonitorJobsDel, getId(), getUid());
      return true;
   }
   case af::Msg::TJobResetErrorHosts:
   {
      for( int b = 0; b < blocksnum; b++) blocks[b]->action( mcgeneral, af::Msg::TBlockResetErrorHosts, NULL, monitoring);
      jobchanged = af::Msg::TMonitorJobsChanged;
      break;
   }
   case af::Msg::TBlockErrorsForgiveTime:
   case af::Msg::TBlockErrorsAvoidHost:
   case af::Msg::TBlockErrorRetries:
   case af::Msg::TBlockTasksMaxRunTime:
   case af::Msg::TBlockErrorsSameHost:
   case af::Msg::TBlockResetErrorHosts:
   case af::Msg::TBlockDependMask:
   case af::Msg::TBlockTasksDependMask:
   case af::Msg::TBlockCommand:
   case af::Msg::TBlockWorkingDir:
   case af::Msg::TBlockFiles:
   case af::Msg::TBlockCmdPost:
   case af::Msg::TBlockHostsMask:
   case af::Msg::TBlockHostsMaskExclude:
   case af::Msg::TBlockMaxRunningTasks:
   case af::Msg::TBlockService:
   case af::Msg::TBlockParser:
   case af::Msg::TBlockNeedMemory:
   case af::Msg::TBlockNeedHDD:
   case af::Msg::TBlockNeedPower:
   case af::Msg::TBlockNeedProperties:
   case af::Msg::TBlockCapacity:
   case af::Msg::TBlockCapacityCoeffMin:
   case af::Msg::TBlockCapacityCoeffMax:
   case af::Msg::TBlockMultiHostMin:
   case af::Msg::TBlockMultiHostMax:
   case af::Msg::TBlockMultiHostWaitMax:
   case af::Msg::TBlockMultiHostWaitSrv:
   {
      int bnum = mcgeneral.getId();
      if( bnum == -1)
      {
         for( int b = 0; b < blocksnum; b++)
         {
            unsigned changes = blocks[b]->action( mcgeneral, type, NULL, monitoring);
            if( jobchanged < changes ) jobchanged = changes;
         }
      }
      else
      {
         if( bnum >= blocksnum ) return false;
         jobchanged = blocks[bnum]->action( mcgeneral, type, NULL, monitoring);
      }
      if( monitoring && jobchanged) monitoring->addJobEvent( jobchanged, getId(), getUid());
      return true;
   }
   default:
   {
      return false;
   }
   }

//   AFCommon::QueueDBUpdateItem( this);
   if( monitoring && jobchanged) monitoring->addJobEvent( jobchanged, getId(), getUid());

   return true;
}

void JobAf::setUserListOrder( int index, bool updateDtabase)
{
   int old_index = userlistorder;
   userlistorder = index;
   if(( index != old_index ) && updateDtabase ) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_userlistorder);
}

void JobAf::checkDepends()
{
   state = state & (~AFJOB::STATE_WAITDEP_MASK);

//printf("JobAf::checkDepends: name1=%s\n", name.toUtf8().data());
   bool depend_local = false;
   bool depend_global = false;

   // check global depends:
   if( hasDependMaskGlobal())
   {
      JobContainerIt jobsIt( jobs);
      for( Job *job = jobsIt.job(); job != NULL; jobsIt.next(), job = jobsIt.job())
      {
         if( job == this ) continue;
//printf("JobAf::checkDepends: name2=%s\n", job->getName().toUtf8().data());
         if(( job->isDone() == false ) && ( checkDependMaskGlobal( job->getName()) ))
         {
            depend_global = true;
//printf("Set.\n");
            break;
         }
      }
   }

   // check local depends:
   if( hasDependMask())
   {
      JobsListIt jobsListIt( user->getJobs());
      for( JobAf *job = jobsListIt.job(); job != NULL; jobsListIt.next(), job = jobsListIt.job())
      {
         if( job == this ) continue;
//printf("JobAf::checkDepends: name2=%s\n", job->getName().toUtf8().data());
         if(( job->isDone() == false ) && ( checkDependMask( job->getName()) ))
         {
            depend_local = true;
//printf("Set.\n");
            break;
         }
      }
   }

   if( depend_local || depend_global ) state = state | AFJOB::STATE_WAITDEP_MASK;
}

af::TaskExec * JobAf::genTask( RenderAf *render, int block, int task, std::list<int> * blocksIds, MonitorContainer * monitoring)
{
   if( state & AFJOB::STATE_OFFLINE_MASK ) return NULL;

   //
   // Recursive dependence check, only if needed
   if( blocksIds )
   {
      std::list<int>::const_iterator bIt = blocksIds->begin();
      std::list<int>::const_iterator end = blocksIds->end();
      while( bIt != end)
      {
         if( block == *bIt)
         {
            log( std::string("Block[") + blocksdata[block]->getName() + "] appears second time while job generating a task.\nJob has a recursive blocks tasks dependence.");
            state = state | AFJOB::STATE_OFFLINE_MASK;
            if( monitoring ) monitoring->addJobEvent( af::Msg::TMonitorJobsChanged, getId(), getUid());
            return NULL;
         }
         bIt++;
      }
      blocksIds->push_back( block);
   }
   //

   if( false == ( blocksdata[block]->getState() & AFJOB::STATE_READY_MASK ) ) return NULL;
   if( task >= blocksdata[block]->getTasksNum() )
   {
      AFERRAR("JobAf::genTask: block[%d] '%s' : %d > number of tasks = %d.",
         block, blocksdata[block]->getName().c_str(), task, blocksdata[block]->getTasksNum())
      return NULL;
   }
   if( false == ( progress->tp[block][task]->state & AFJOB::STATE_READY_MASK) ) return NULL;

   if( false == blocks[block]->canRun( render)) return NULL;

   if( blocks[block]->tasks[task]->avoidHostsCheck( render->getName())) return NULL;

   //
   // Check block tasks dependence: Get tasks depend mask, if any exists:
   if( blocksdata[block]->hasTasksDependMask() )
   {
//printf("JobAf::genTask: hasTasksDependMask:\n");
      bool dependsnotdone = false;
      std::list<int> blocksIdsBranch;
      if( blocksIds)
      {
         blocksIdsBranch = *blocksIds;
         blocksIds = &blocksIdsBranch;
      }
//{printf("Ids =");for( std::list<int>::const_iterator it = blocksIds->begin(); it != blocksIds->end(); it++) printf(" %d",*it);printf("\n");}
      for( int b = 0; b < blocksnum; b++)
      {
         if( b == block ) continue;
         if( blocksdata[block]->checkTasksDependMask( blocksdata[b]->getName()) == false ) continue;

         int fpt = blocksdata[block]->getFramePerTask();
         int fpt_dep = blocksdata[b]->getFramePerTask();
         if(( fpt == 0 ) || ( fpt_dep == 0 )) continue;

         int firstdependframe, lastdependframe, firstdependtask, lastdependtask;
         if( fpt > 0 )
         {
            firstdependframe = task * fpt;
            lastdependframe  = firstdependframe + fpt - 1;
         }
         else
         {
            firstdependframe = task / (-fpt);
            lastdependframe  = ( task + 1 ) / (-fpt);
         }
         if( fpt_dep > 0)
         {
            firstdependtask  = firstdependframe / fpt_dep;
            lastdependtask   = lastdependframe  / fpt_dep;
         }
         else
         {
            firstdependtask  = firstdependframe * (-fpt_dep);
            lastdependtask   = ( lastdependframe + 1 ) * (-fpt_dep) - 1;
         }

         if( firstdependtask >= blocksdata[b]->getTasksNum()) firstdependtask = blocksdata[b]->getTasksNum() - 1;
         if( lastdependtask  >= blocksdata[b]->getTasksNum())  lastdependtask = blocksdata[b]->getTasksNum() - 1;
         if( firstdependtask < 0 ) firstdependtask = 0;
         if( lastdependtask  < 0 )  lastdependtask = 0;
//printf("Dep['%s': #%d '%s'] = %d - %d (%d, %d): %d - %d)\n", blocksdata[block]->getName().toUtf8().data(), task, blocksdata[b]->getName().toUtf8().data(), firstdependframe, lastdependframe, fpt, fpt_dep, firstdependtask, lastdependtask);

         for( int t = firstdependtask; t <= lastdependtask; t++)
         {
//printf("Dep['%s':%d-'%s']: checking '%s':%d - %s\n", blocksdata[block]->getName().toUtf8().data(), task, blocksdata[b]->getName().toUtf8().data(), blocksdata[b]->getName().toUtf8().data(), t, (progress->tp[b][t]->state & AFJOB::STATE_DONE_MASK) ? "DONE" : "NOT Done");
            if( progress->tp[b][t]->state & AFJOB::STATE_DONE_MASK ) continue;
            af::TaskExec * task_ptr = genTask( render, b, t, (t == firstdependtask ? blocksIds : NULL), monitoring);
            if( state & AFJOB::STATE_OFFLINE_MASK )
            {
               if( task_ptr ) delete task_ptr;
               return NULL;
            }
//if( task_ptr) printf("Dep['%s':%d-'%s']: starting '%s':%d\n", blocksdata[block]->getName().toUtf8().data(), task, blocksdata[b]->getName().toUtf8().data(), blocksdata[b]->getName().toUtf8().data(), t);
            if( task_ptr ) return task_ptr;
            if( dependsnotdone == false ) dependsnotdone = true;
         }
      }
      if( dependsnotdone ) return NULL;
   }

   return blocksdata[block]->genTask( task);
}

bool JobAf::solve( RenderAf *render, MonitorContainer * monitoring)
{
   if( isLocked() ) return false;
//printf("JobAf::solve( RenderAf *render): %s - %s\n", name.toUtf8().data(), render->getName().toUtf8().data());
// check validness:
   if( blocksnum < 1)
   {
      AFERROR("JobAf::solve: job has no blocks.")
      return false;
   }

// check priority:
   if( priority == 0 ) return false;

// check job state:
   if(            state & AFJOB::STATE_OFFLINE_MASK ) return false;
   if( false == ( state & AFJOB::STATE_READY_MASK  )) return false;

// check maximum hosts:
   if(( maxrunningtasks >= 0 ) && ( getRunningTasksNumber() >= maxrunningtasks )) return false;

// check blocks with enough capacity
   bool enoughCapacity = false;
   for( int b = 0; b < blocksnum; b++)
   {
      if( render->hasCapacity( blocksdata[b]->getCapMinResult()))
      {
         enoughCapacity = true;
         break;
      }
   }
   if( false == enoughCapacity) return false;


// check hosts mask:
   if( false == checkHostsMask( render->getName())) return false;
// check exclude hosts mask:
   if( false == checkHostsMaskExclude( render->getName())) return false;
// check needed os:
   if( false == checkNeedOS( render->getHost().os)) return false;
// check needed properties:
   if( false == checkNeedProperties( render->getHost().properties)) return false;

// search for ready task:
   for( int b = 0; b < blocksnum; b++)
   {
      if( false == ( blocksdata[b]->getState() & AFJOB::STATE_READY_MASK )) continue;

      int numtasks = blocksdata[b]->getTasksNum();
      for( int t = 0; t < numtasks; t++)
      {
         if( false == ( progress->tp[b][t]->state & AFJOB::STATE_READY_MASK )) continue;
         std::list<int> blocksIds;
         af::TaskExec *taskexec = genTask( render, b, t, &blocksIds, monitoring);
         // Job may became paused, if recursion during task generation detected:
         if( state & AFJOB::STATE_OFFLINE_MASK )
         {
            if( taskexec ) delete taskexec;
            return false;
         }
         // No task was generated:
         if( taskexec == NULL ) continue;

         // Job successfully solved (produced a task)
         taskexec->setJobName( name);
         taskexec->setUserName( username);
         listeners.process( *taskexec);
         blocks[taskexec->getBlockNum()]->startTask( taskexec, render, monitoring);
         // If job was not started it became started
         if( time_started == 0 )
         {
            time_started = time(NULL);
            log("Started.");
            AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_time_started);
         }

         return true;
      }
   }
   return false;
}

void JobAf::updateTaskState( const af::MCTaskUp& taskup, RenderContainer * renders, MonitorContainer * monitoring)
{
   if( false == checkBlockTaskNumbers( taskup.getNumBlock(), taskup.getNumTask(), "updateTaskState")) return;
   bool errorHost = false;
   blocks[taskup.getNumBlock()]->tasks[taskup.getNumTask()]->updateState( taskup, renders, monitoring, errorHost);
   if( errorHost) blocks[taskup.getNumBlock()]->errorHostsAppend( taskup.getNumTask(), taskup.getClientId(), renders);
}

void JobAf::refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
//printf("JobAf::refresh: \"%s\"\n", getName().toUtf8().data());
   RenderContainer * renders = (RenderContainer*)pointer;

   if( deletion )
   {
//printf("JobAf::refresh: deletion: runningtaskscounter = %d\n", runningtaskscounter);
      for( int b = 0; b < blocksnum; b++)
         blocks[b]->refresh( currentTime, renders, monitoring);
      if( getRunningTasksNumber() == 0 ) setZombie( NULL, monitoring);
//printf("JobAf::refresh: deletion: runningtaskscounter = %d\n", runningtaskscounter);
   }
   if( isLocked() ) return;

   // for database and monitoring
   uint32_t old_state = state;
   uint32_t jobchanged = 0;

   // check job dependences
   checkDepends();

   //check wait time
   {
      bool wasWaiting = state & AFJOB::STATE_WAITTIME_MASK;
      if( time_wait > currentTime ) state = state |   AFJOB::STATE_WAITTIME_MASK;
      else                          state = state & (~AFJOB::STATE_WAITTIME_MASK);
      bool nowWaining = state & AFJOB::STATE_WAITTIME_MASK;
      if( wasWaiting != nowWaining) jobchanged = af::Msg::TMonitorJobsChanged;
   }

   //
   // Update blocks (blocks will uptate its tasks):
   for( int b = 0; b < blocksnum; b++)
      if( blocks[b]->refresh( currentTime, renders, monitoring))
         jobchanged = af::Msg::TMonitorJobsChanged;

   //
   // job state calculation
   state = state |   AFJOB::STATE_DONE_MASK;
   state = state & (~AFJOB::STATE_RUNNING_MASK);
   state = state & (~AFJOB::STATE_ERROR_MASK);
   state = state & (~AFJOB::STATE_READY_MASK);
   state = state & (~AFJOB::STATE_SKIPPED_MASK);

   for( int b = 0; b < blocksnum; b++)
   {
      uint32_t state_block = blocksdata[b]->getState();
      state  = state | (state_block &   AFJOB::STATE_RUNNING_MASK   );
      state  = state | (state_block &   AFJOB::STATE_ERROR_MASK     );
      state  = state | (state_block &   AFJOB::STATE_READY_MASK     );
      state  = state | (state_block &   AFJOB::STATE_SKIPPED_MASK   );
      state  = state & (state_block | (~AFJOB::STATE_DONE_MASK)     );
   }

   if( state & AFJOB::STATE_WAITDEP_MASK  ) state = state & (~AFJOB::STATE_READY_MASK);
   if( state & AFJOB::STATE_WAITTIME_MASK ) state = state & (~AFJOB::STATE_READY_MASK);

   if( state & AFJOB::STATE_DONE_MASK )
   {
      /// if job was not done, but now is done, we set job time_done
      if(( old_state & AFJOB::STATE_DONE_MASK) == false )
      {
         time_done = currentTime;
         if( time_started == 0 )
         {
            time_started = time_done;
            log("Started.");
         }
         log("Done.");
         jobchanged = af::Msg::TMonitorJobsChanged;
         AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_time_done);
      }
   }
   else
      time_done = 0;

   // Reset started time if job was started (for "some" time) but now no tasks are running or done
//   static int check_started_time = 11; // This peroid needed not to update started time every cycle
   if(( time_started != 0 ) &&
//      ( currentTime - time_started > check_started_time  ) &&
      ( false == (state & AFJOB::STATE_RUNNING_MASK)     ) &&
      ( false == (state & AFJOB::STATE_DONE_MASK)        )  )
   {
      // Search if the job has at least one done task
      bool has_done_tasks = false;
      for( int b = 0; b < blocksnum; b++ )
      {
         if( blocksdata[b]->getProgressTasksDone() > 0 )
         {
            has_done_tasks = true;
            break;
         }
      }
      // If the job has done task(s) we not reset started time in any case
      if( false == has_done_tasks )
      {
         time_started = currentTime;
//         jobchanged = af::Msg::TMonitorJobsChanged;
//         AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_time_started);
      }
   }

   // If it is no job monitoring, job just came to server and it is first it refresh,
   // so no change event and database storing needed

   if( state != old_state )
   {
      jobchanged = af::Msg::TMonitorJobsChanged;
      if( monitoring ) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
   }

   // Check age and delete if life finished:
   if( id != AFJOB::SYSJOB_ID ) // skip system job
   {
      int result_lifetime = lifetime;
      if( result_lifetime < 0 ) result_lifetime = user->getJobsLifeTime(); // get default value from user
      if((result_lifetime > 0) && ((currentTime - time_creation) > result_lifetime))
      {
         log( std::string("Life %1 finished.") + af::time2strHMS( result_lifetime, true));
         user->appendLog( std::string("Job \"") + name + "\" life " + af::time2strHMS( result_lifetime, true) + " finished.");
         setZombie( renders, monitoring);
         jobchanged = af::Msg::TMonitorJobsDel, getId(), getUid();
      }
   }

   if(( monitoring ) &&  ( jobchanged )) monitoring->addJobEvent( jobchanged, getId(), getUid());
}

void JobAf::skipTasks( const af::MCTasksPos &taskspos, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("JobAf::skipTasks:\n");
   tasks_Skip_Restart( taskspos, false,renders, monitoring);
//printf("JobAf::skipTasks: tasks skipped.\n");
}

void JobAf::restartTasks( const af::MCTasksPos &taskspos, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("JobAf::restartTasks:\n");
   tasks_Skip_Restart( taskspos, true, renders, monitoring);
//printf("JobAf::restartTasks: tasks restarted.\n");
}

void JobAf::tasks_Skip_Restart( const af::MCTasksPos &taskspos, bool restart, RenderContainer * renders, MonitorContainer * monitoring)
{
   AFCommon::QueueDBUpdateTask_begin();
   for( int p = 0; p < taskspos.getCount(); p++)
   {
      int b = taskspos.getNumBlock(p);
      if( b >= blocksnum)
      {
         AFERRAR("JobAf::skipTasks: b >= blocksnum ( %d >= %d )", b, blocksnum)
         continue;
      }
      bool has_tasks = taskspos.hasTasks();
      int start, end;
      if( has_tasks )
      {
         start = taskspos.getNumTask( p);
         if( start >= blocksdata[b]->getTasksNum())
         {
            AFERRAR("JobAf::skipTasks: taskspos.getNumTask() >= numTasks , ( %d >= %d )", start, blocksdata[b]->getTasksNum())
            continue;
         }
         end = start+1;
      }
      else
      {
         start = 0;
         end = blocksdata[b]->getTasksNum();
      }

      std::string message;
      if( restart) message = "Restart request by ";
      else         message = "Skip request by ";
      message += taskspos.getUserName() + '@' + taskspos.getHostName() + ' ' + taskspos.getMessage();

      for( int t = start; t < end; t++)
      {
         if( restart) blocks[b]->tasks[t]->restart( false, message, renders, monitoring);
         else         blocks[b]->tasks[t]->skip( message, renders, monitoring);
      }
   }
   AFCommon::QueueDBUpdateTask_end();
}

void JobAf::restartAllTasks( bool onlyRunning, const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
{
   AFCommon::QueueDBUpdateTask_begin();
   for( int b = 0; b < blocksnum; b++)
   {
      int numtasks = blocksdata[b]->getTasksNum();
      for( int t = 0; t < numtasks; t++)
         blocks[b]->tasks[t]->restart( onlyRunning, message, renders, monitoring);
   }
   AFCommon::QueueDBUpdateTask_end();
}

void JobAf::restartErrors( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
{
   for( int b = 0; b < blocksnum; b++)
   {
      int numtasks = blocksdata[b]->getTasksNum();
      for( int t = 0; t < numtasks; t++)
         blocks[b]->tasks[t]->restartError( message, renders, monitoring);
   }
}

void JobAf::writeProgress( af::Msg &msg)
{
   msg.set( af::Msg::TJobProgress, progress);
}

const std::list<std::string> & JobAf::getTaskLog( int block, int task)
{
   static const std::list<std::string> emptylog;
   if( false == checkBlockTaskNumbers( block, task, "getTaskLog")) return emptylog;
   return blocks[block]->tasks[task]->getLog();
}

af::TaskExec * JobAf::generateTask( int block, int task)
{
   if( false == checkBlockTaskNumbers( block, task, "generateTask")) return NULL;
   return blocks[block]->data->genTask( task);
}

const std::string JobAf::getErrorHostsListString() const
{
   std::string str("Job \"");
   str += name + "\" error hosts:";
   for( int block = 0; block < blocksnum; block++) blocks[block]->getErrorHostsListString( str);
   return str;
}

const std::string JobAf::getErrorHostsListString( int b, int t) const
{
   if( false == checkBlockTaskNumbers( b, t, "getErrorHostsList"))
   {
      return std::string("Invalid task[") + af::itos(b) + "][" + af::itos(t) + "].";
   }
   std::string str = blocks[b]->tasks[t]->getErrorHostsListString();
   if( str.empty()) str = "The task has no error hosts.";
   return str;
}

bool JobAf::checkBlockTaskNumbers( int BlockNum, int TaskNum, const char * str) const
{
   if( BlockNum >= blocksnum)
   {
      if( str ) AFERRAR("JobAf::checkBlockTaskNumbers: %s: numblock >= blocksnum ( %d >= %d )", str, BlockNum, blocksnum)
      else      AFERRAR("JobAf::checkBlockTaskNumbers: numblock >= blocksnum ( %d >= %d )", BlockNum, blocksnum)
      return false;
   }
   if( TaskNum >= blocksdata[BlockNum]->getTasksNum())
   {
      if( str ) AFERRAR("JobAf::checkBlockTaskNumbers: %s: numtask >= numTasks ( %d >= %d )", str, TaskNum, blocksdata[BlockNum]->getTasksNum())
      else      AFERRAR("JobAf::checkBlockTaskNumbers: numtask >= numTasks ( %d >= %d )", TaskNum, blocksdata[BlockNum]->getTasksNum())
      return false;
   }
   return true;
}

bool JobAf::getTaskStdOut( const af::MCTaskPos &taskpos, MsgAf *msg, std::string & filename, RenderContainer * renders)
{
//printf("JobAf::getTaskStdOut:\n");
   if( false == checkBlockTaskNumbers( taskpos.getNumBlock(), taskpos.getNumTask(), "getTaskStdOut"))
   {
      msg->setString("JobAf::getTaskStdOut: invalid job and task numbers");
      return false;
   }
   return blocks[taskpos.getNumBlock()]->tasks[taskpos.getNumTask()]->getOutput( taskpos.getNumber(), msg, filename, renders);
}

void JobAf::listenOutput( af::MCListenAddress & mclisten, RenderContainer * renders)
{
//printf("JobAf::listenOutput: (%s) ", onoff?"ON":"OFF");address->stdOut();printf(", jobid=%d:\n", id);
   if( listeners.process( mclisten) == false ) return;

   if( mclisten.justTask())
   {
      checkBlockTaskNumbers( mclisten.getNumBlock(), mclisten.getNumTask(), "listenOutput");
      blocks[mclisten.getNumBlock()]->tasks[mclisten.getNumTask()]->listenOutput( mclisten, renders);
   }
   else
   {
      for( int b = 0; b < blocksnum; b++)
         for( int t = 0; t < blocksdata[b]->getTasksNum(); t++)
            blocks[b]->tasks[t]->listenOutput( mclisten, renders);
   }
}

void JobAf::log( const std::string & message)
{
   joblog.push_back( af::time2str() + " : " + message);
   while( joblog.size() > af::Environment::getJobLogLinesMax() ) joblog.pop_front();
}

int JobAf::calcWeight() const
{
//printf("JobAf::calcWeight: '%s' runningtaskscounter=%d\n", name.toUtf8().data(), runningtaskscounter);

   int weight = Job::calcWeight();
//printf("JobAf::calcWeight: Job::calcWeight: %d bytes\n", weight);
   weight += sizeof(JobAf) - sizeof( Job);

   if( progress != NULL) progressWeight = progress->calcWeight();
   weight += progressWeight;

   logsWeight = 0;
   for( std::list<std::string>::const_iterator it = joblog.begin(); it != joblog.end(); it++)
      logsWeight += af::weigh( *it);

   for( int b = 0; b < blocksnum; b++)
   {
      weight += blocks[b]->calcWeight();
      blackListsWeight += blocks[b]->blackListWeight();
      logsWeight += blocks[b]->logsWeight();
   }

   weight += listeners.calcWeight();

   weight += blackListsWeight;
   weight += logsWeight;

//printf("JobAf::calcWeight: %d bytes ( sizeof JobAf = %d)\n", weight, sizeof(JobAf));
   return weight;
}
