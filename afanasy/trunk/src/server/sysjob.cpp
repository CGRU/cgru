#include "sysjob.h"

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"

#include "../libafsql/dbjobprogress.h"

#include "rendercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   COMMAND    ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

SysCmd::SysCmd( const QString & Command, const QString & WorkingDirectory, const QString & UserName, const QString & JobName):
   command( Command), workingdirectory( WorkingDirectory), username( UserName), jobname( JobName) {}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   TASK    //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

SysTask::SysTask( af::TaskExec * taskexec, SysCmd * SystemCommand, const Block * block, int taskNumber):
   Task( block, &taskProgress, taskNumber),
   syscmd( SystemCommand),
   birthtime(0)
{
   progress = &taskProgress;
}

SysTask::~SysTask()
{
// Delete system command data:
   delete syscmd;
}

QString const SysTask::getInfo(bool full) const
{
   QString info;
   info += QString("#%1:").arg( getNumber(), 3);
   info += QString(" %1").arg( af::state2str( progress->state), 4);
   info += QString(": %1").arg( syscmd->command);
   return info;
}

void SysTask::monitor( MonitorContainer * monitoring) const {}
void SysTask::updateDatabase() const {}
void SysTask::writeTaskOutput( const af::MCTaskUp& taskup) const {}

void SysTask::log( const QString &message)
{
   SysBlock::logCmdPost( QString("#%1: %2: %3: \"%4\":\n%5")
                         .arg(getNumber()).arg(message).arg(syscmd->username).arg(syscmd->jobname).arg(syscmd->command));
}

void SysTask::appendSysJobLog( const QString &message)
{
   SysJob::appendLog( QString("Task[%1]: %2: %3: \"%4\":\n%5")
                         .arg(getNumber()).arg(message).arg(syscmd->username).arg(syscmd->jobname).arg(syscmd->command));
}

void SysTask::start( af::TaskExec * taskexec, int * runningtaskscounter, RenderAf * render, MonitorContainer * monitoring)
{
//printf("SysTask::start:\n");
   taskexec->setCommand(      syscmd->command               );
   taskexec->setName(         syscmd->command.split(' ')[0] );
   taskexec->setUserName(     syscmd->username              );
   taskexec->setJobName(      syscmd->jobname               );
   taskexec->setWDir(         syscmd->workingdirectory      );
   taskexec->setTaskNumber(   getNumber()                   );

   Task::start( taskexec, runningtaskscounter, render, monitoring);
}

void SysTask::refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring, int & errorHostId)
{
//printf("SysTask::refresh:\n");
   Task::refresh( currentTime, renders, monitoring, errorHostId);
   if( birthtime == 0 ) birthtime = currentTime;
   if((currentTime - birthtime > af::Environment::getSysJobTaskLife() ) && (isReady()))
   {
      QString message = QString( QString("Error: Task age(%1) > %2").arg(currentTime - birthtime).arg(af::Environment::getSysJobTaskLife()));
      log( message);
      // Store error in job log
      appendSysJobLog( message);
      progress->state = AFJOB::STATE_ERROR_MASK;
   }
//stdOut();
}

void SysTask::updateState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring, bool & errorHost)
{
//printf("SysTask::updateState: "); taskup.stdOut( true);
   Task::updateState( taskup, renders, monitoring, errorHost);
//stdOut();

   // Store error messages and logs:

   QString message;
   switch ( taskup.getStatus())
   {
      case af::TaskExec::UPWarning:                   message = "Warning";             break;
      case af::TaskExec::UPFailedToStart:             message = "Failed to start";     break;
      case af::TaskExec::UPFinishedCrash:             message = "Finished crashed";    break;
      case af::TaskExec::UPFinishedParserError:       message = "Parser error";        break;
      case af::TaskExec::UPFinishedParserBadResult:   message = "Parser bad result";   break;
      case af::TaskExec::UPFinishedError:             message = "Finished error";      break;
      case af::TaskExec::UPEject:                     message = "Ejected";             break;
      case af::TaskExec::UPRenderDeregister:          message = "Render deregistered"; break;
      case af::TaskExec::UPRenderExit:                message = "Render exit";         break;
      case af::TaskExec::UPRenderZombie:              message = "Render zombie";       break;
   default: return;
   }

   // Get render name:
   if( taskup.getClientId() != 0 )
   {
      RenderContainerIt rendersIt( renders);
      RenderAf * render = rendersIt.getRender( taskup.getClientId());
      if( render ) message += QString(" on '%1'").arg( render->getName());
   }

   // Store error in job log
   appendSysJobLog( message);

   // Store error task output in task log
   if( taskup.getDataLen() > 0)
   {
      message = QString("Error task output:");
      message += "\n";
      message += "=======================================================";
      message += "\n";
      message += QString::fromUtf8( taskup.getData(), taskup.getDataLen());
      message += "\n";
      message += "=======================================================";
      SysBlock::logCmdPost(message);
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   BLOCK    /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

Task * SysBlock::task = NULL;

SysBlock::SysBlock( af::Job * blockJob, af::BlockData * blockData, af::JobProgress * progress, QStringList * log):
   Block( blockJob, blockData, progress, log)
{
//printf("SysBlock::SysBlock:\n");
   task = tasks[0];
}

SysBlock::~SysBlock()
{
   for( std::list<SysCmd *>::iterator it = commands.begin(); it != commands.end(); it++) delete *it;
   for( std::list<SysTask*>::iterator it = systasks.begin(); it != systasks.end(); it++) delete *it;
}

void SysBlock::addCommand( SysCmd * syscmd)
{
   commands.push_back( syscmd);
}

void SysBlock::startTask( af::TaskExec * taskexec, RenderAf * render, MonitorContainer * monitoring)
{
//printf("SysBlock::startTask:\n");
   taskexec->setBlockName( data->getName());
   SysTask * systask = getReadySysTask();

   // Add new ready task:
   if( systask == NULL ) systask = addTask( taskexec);

   if( systask == NULL ) return;

   systask->start( taskexec, data->getRunningTasksCounter(), render, monitoring);
}

SysTask * SysBlock::getReadySysTask() const
{
   for( std::list<SysTask*>::const_iterator it = systasks.begin(); it != systasks.end(); it++)
      if( (*it)->isReady() ) return *it;
   return NULL;
}

SysTask * SysBlock::addTask( af::TaskExec * taskexec)
{
//printf("SysBlock::addTask:\n");
   if( commands.size() == 0)
   {
      AFERROR("SysBlock::addTask: commands.size() == 0\n");
      return NULL;
   }

   // Get the first command:
   SysCmd * command = commands.front();

   // Get the smallest task number:
   int number = 0;
   bool founded;
   for( ; number < af::Environment::getSysJobTasksMax(); number++)
   {
      founded = true;
      for( std::list<SysTask*>::iterator it = systasks.begin(); it != systasks.end(); it++)
      {
         if((*it)->getNumber() == number)
         {
            founded = false;
            break;
         }
      }
      if( founded ) break;
   }
   if( false == founded )
   {
      AFERRAR("SysBlock::addTask: Can't find free task number (max=%d).\n", af::Environment::getSysJobTasksMax());
      log(QString("Can't find task number (max=%1)").arg( af::Environment::getSysJobTasksMax()));
      return NULL;
   }

   // Create system task:
   SysTask * systask = new SysTask( taskexec, command, this, number);
   // Delete first command pointer ( command data will be deleted in task destructor):
   commands.pop_front();

   // Add system task:
   systasks.push_back( systask);

   return systask;
}

void SysBlock::errorHostsAppend( int task, int hostId, RenderContainer * renders)
{
   RenderContainerIt rendersIt( renders);
   RenderAf* render = rendersIt.getRender( hostId);
   if( render == NULL ) return;
   if( Block::errorHostsAppend( render->getName())) log( render->getName() + " - AVOIDING HOST !");
   SysTask * systask = getTask( task, "errorHostsAppend");
   if( systask) systask->errorHostsAppend( render->getName());
}

void SysBlock::updateTaskState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("SysBlock::updateTaskState:\n");
   SysTask * systask = getTask( taskup.getNumTask(), "updateTaskState");
   if( systask == NULL ) return;
   bool errorHost = false;
   systask->updateState( taskup, renders, monitoring, errorHost);
   if( errorHost) errorHostsAppend( 0, taskup.getClientId(), renders);
}

bool SysBlock::refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("SysBlock::refresh:\n");

   for( std::list<SysTask*>::iterator it = systasks.begin(); it != systasks.end(); it++)
   {
      int errorHostId = -1;
      (*it)->refresh( currentTime, renders, monitoring, errorHostId);
      if( errorHostId != -1 ) errorHostsAppend( 0, errorHostId, renders);
   }

   deleteFinishedTasks();

   return Block::refresh( currentTime, renders, monitoring);
}

void SysBlock::deleteFinishedTasks()
{
   for( std::list<SysTask*>::iterator it = systasks.begin(); it != systasks.end();)
   {
      if(( false == (*it)->isRunning()) && ((*it)->isError() || (*it)->isDone()))
      {
         delete *it;
         it = systasks.erase( it);
      }
      else it++;
   }
}

SysTask * SysBlock::getTask( int tasknum, const char * errorMessage)
{
   for( std::list<SysTask*>::iterator it = systasks.begin(); it != systasks.end(); it++)
      if( tasknum == (*it)->getNumber()) return *it;
   if( errorMessage ) { AFERRAR("SysJob::getTask: %s: Invalid task number = %d\n", errorMessage, tasknum);}
   else { AFERRAR("SysJob::getTask: Invalid task number = %d\n", tasknum);}
}

void SysBlock::errorHostsReset()
{
//printf("SysBlock::errorHostsReset:\n");
   Block::errorHostsReset();
   for( std::list<SysTask*>::iterator it = systasks.begin(); it != systasks.end(); it++) (*it)->errorHostsReset();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   JOB    ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

SysBlock * SysJob::block_cmdpost = NULL;
SysJob * SysJob::sysjob = NULL;

SysJob::SysJob( int flags):
   JobAf( AFJOB::SYSJOB_ID)
{
   sysjob = this;

   if( flags != New ) return;

   name              = AFJOB::SYSJOB_NAME;
   username          = AFJOB::SYSJOB_USERNAME;
   priority          = AFGENERAL::DEFAULT_PRIORITY;
   maxrunningtasks   = AFGENERAL::MAXRUNNINGTASKS;

   blocksnum = 1;
   blocksdata = new af::BlockData*[blocksnum];
   for( int b = 0; b < blocksnum; b++)
   {
      blocksdata[b] = new SysBlockData( b, id);
   }

   progress = new afsql::DBJobProgress( this);

   construct();
   fromdatabase = false;

   printf("System job constructed.\n");
}

SysJob::~SysJob()
{
}

void SysJob::dbDelete( QStringList  * queries) const
{
   AFERROR("Trying to delete system job from database.\n");
}

void SysJob::setZombie( RenderContainer * renders, MonitorContainer * monitoring)
{
   AFERROR("Trying to make system job a zomibe.\n");
}

Block * SysJob::createBlock( int numBlock)
{
   switch( numBlock)
   {
   case BlockPostCmdIndex:
   {
      block_cmdpost = new SysBlock( this, blocksdata[numBlock], progress, &joblog);
      return block_cmdpost;
   }
   default:
      printf("SysJob::createBlock: Invalid block number = %d\n", numBlock);
   }
}

void SysJob::addCommand( const QString & Command, const QString & WorkingDirectory, const QString & UserName, const QString & JobName)
{
   block_cmdpost->addCommand( new SysCmd( Command, WorkingDirectory, UserName, JobName));
}

bool SysJob::solve( RenderAf *render, MonitorContainer * monitoring)
{
   if( block_cmdpost->getReadySysTask() == NULL )
   {
      if( block_cmdpost->getNumCommands() < 1 ) return false;
      if( block_cmdpost->getNumSysTasks() >= af::Environment::getSysJobTasksMax() ) return false;
   }
//printf("SysJob::solve:\n");
   return JobAf::solve( render, monitoring);
}

void SysJob::updateTaskState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("SysJob::updateTaskState:\n");
//   JobAf::updateTaskState( taskup, renders, monitoring)

   switch( taskup.getNumBlock())
   {
   case BlockPostCmdIndex:
   {
      block_cmdpost->updateTaskState( taskup, renders, monitoring);
      break;
   }
   default:
      AFERRAR("SysJob::updateTaskState: Invalid block number = %d\n", taskup.getNumBlock());
   }
}

void SysJob::refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
//printf("SysJob::refresh:\n");
   JobAf::refresh( currentTime, pointer, monitoring);
}

bool SysJob::action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring)
{
//printf("SysJob::action:\n");

   switch( type)
   {
      case af::Msg::TJobAnnotate:
      case af::Msg::TJobHostsMask:
      case af::Msg::TJobHostsMaskExclude:
      case af::Msg::TJobDependMask:
      case af::Msg::TJobDependMaskGlobal:
      case af::Msg::TJobMaxRunningTasks:
      case af::Msg::TJobWaitTime:
      case af::Msg::TJobPriority:
      case af::Msg::TJobNeedOS:
      case af::Msg::TJobNeedProperties:
      case af::Msg::TJobStart:
      case af::Msg::TJobPause:
      case af::Msg::TJobResetErrorHosts:
      case af::Msg::TBlockErrorsAvoidHost:
      case af::Msg::TBlockErrorRetries:
      case af::Msg::TBlockTasksMaxRunTime:
      case af::Msg::TBlockErrorsSameHost:
      case af::Msg::TBlockErrorsForgiveTime:
      case af::Msg::TBlockResetErrorHosts:
      case af::Msg::TBlockDependMask:
      case af::Msg::TBlockTasksDependMask:
      case af::Msg::TBlockWorkingDir:
      case af::Msg::TBlockHostsMask:
      case af::Msg::TBlockHostsMaskExclude:
      case af::Msg::TBlockMaxRunningTasks:
      case af::Msg::TBlockNeedMemory:
      case af::Msg::TBlockNeedHDD:
      case af::Msg::TBlockNeedPower:
      case af::Msg::TBlockNeedProperties:
      case af::Msg::TBlockCapacity:
      case af::Msg::TBlockCapacityCoeffMin:
      case af::Msg::TBlockCapacityCoeffMax:
         return JobAf::action( mcgeneral, type, pointer, monitoring);
   }

   return true;
}

void SysJob::appendLog( const QString message)
{
   sysjob->log( message);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   BLOCK DATA   //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SysBlockData::SysBlockData( int BlockNum, int JobId):
   afsql::DBBlockData( BlockNum, JobId)
{
   initDefaults();

   capacity = af::Environment::getTaskDefaultCapacity();

   name = "post_commands";
   service = AFJOB::SYSJOB_BLOCKSERVICE;

   tasksmaxruntime = AFJOB::SYSJOB_TASKMAXRUNTIME;

/// Maximum number or errors on same host for block NOT to avoid host
   errors_avoidhost = AFJOB::SYSJOB_ERRORS_AVIODHOST;
/// Maximum number or errors on same host for task NOT to avoid host
   errors_tasksamehost = AFJOB::SYSJOB_ERRORS_TASKSAMEHOST;
/// Maximum number of errors in task to retry it automatically
   errors_retries = AFJOB::SYSJOB_ERRORS_RETRIES;
/// Time from last error to remove host from error list
   errors_forgivetime = AFJOB::SYSJOB_ERRORS_FORGIVETIME;

   tasksnum = 1;

   tasksdata = new af::TaskData*[tasksnum];
   for( int t = 0; t < tasksnum; t++)
   {
      tasksdata[t] = new SysTaskData;
   }
}

SysBlockData::~SysBlockData()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   TASK DATA   //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

SysTaskData::SysTaskData():
   afsql::DBTaskData()
{
   name = "Dummy task. See all tasks logs here.";
}

SysTaskData::~SysTaskData()
{
}
