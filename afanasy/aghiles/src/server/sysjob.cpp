#include "sysjob.h"

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"

#include "../libafsql/dbjobprogress.h"

#include "afcommon.h"
#include "afcommon.h"
#include "monitorcontainer.h"
#include "rendercontainer.h"
#include "sysjob_cmdpost.h"
#include "sysjob_wol.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   COMMAND    ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

SysCmd::SysCmd( const std::string & Command, const std::string & WorkingDirectory, const std::string & UserName, const std::string & JobName):
   command( Command), workingdirectory( WorkingDirectory), username( UserName), jobname( JobName) {}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   TASK    //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

SysTask::SysTask( af::TaskExec * taskexec, SysCmd * SystemCommand, Block * block, int taskNumber):
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

std::string const SysTask::getInfo(bool full) const
{
   std::string info = "#";
   info += af::itos( getNumber()) + ": ";
   info += af::state2str( progress->state) + ": ";
   info += syscmd->command;
   return info;
}

void SysTask::monitor( MonitorContainer * monitoring) const {}
void SysTask::updateDatabase() const {}
void SysTask::writeTaskOutput( const af::MCTaskUp& taskup) const {}

void SysTask::appendLog( const std::string & message)
{
   ((SysBlock*)(block))->appendTaskLog( std::string("#") + af::itos( getNumber()) + ": " + message + ": "
                         + syscmd->username + ": \"" + syscmd->jobname + "\":\n"
                         + syscmd->command);
}

void SysTask::appendSysJobLog( const std::string & message)
{
   SysJob::appendJobLog( std::string("Task[") + af::itos( getNumber()) + "]: " + message + ": "
                      + syscmd->username + ": \"" + syscmd->jobname + "\":\n"
                      + syscmd->command);
}

void SysTask::start( af::TaskExec * taskexec, int * runningtaskscounter, RenderAf * render, MonitorContainer * monitoring)
{
   std::string name = syscmd->command.substr( 0, AFJOB::SYSJOB_TASKSNAMEMAX);
   size_t space = name.find(' ', 1);
   if( space != std::string::npos )
      name = name.substr( 0, space);

   taskexec->setCommand(      syscmd->command            );
   taskexec->setName(         name                       );
   taskexec->setUserName(     syscmd->username           );
   taskexec->setJobName(      syscmd->jobname            );
   taskexec->setWDir(         syscmd->workingdirectory   );
   taskexec->setTaskNumber(   getNumber()                );

   Task::start( taskexec, runningtaskscounter, render, monitoring);
}

void SysTask::refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring, int & errorHostId)
{
//printf("SysTask::refresh:\n");
   Task::refresh( currentTime, renders, monitoring, errorHostId);
   if( birthtime == 0 ) birthtime = currentTime;
   if((currentTime - birthtime > af::Environment::getSysJobTaskLife() ) && (isReady()))
   {
      std::string message = std::string("Error: Task age(") + af::itos( currentTime - birthtime) + ") > " + af::itos( af::Environment::getSysJobTaskLife());
      appendLog( message);
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

   std::string message;
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
      if( render ) message += std::string(" on \"") + render->getName() + "\"";
   }

   // Store error in job log
   appendSysJobLog( message);

   // Store error task output in task log
   if( taskup.getDataLen() > 0)
   {
      message = "Error task output:";
      message += "\n";
      message += "=======================================================";
      message += "\n";
      message += std::string( taskup.getData(), taskup.getDataLen());
      message += "\n";
      message += "=======================================================";
      ((SysBlock*)(block))->appendTaskLog(message);
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   BLOCK    /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Task * SysBlock::task = NULL;

SysBlock::SysBlock( JobAf * blockJob, af::BlockData * blockData, af::JobProgress * progress, std::list<std::string> * log):
   Block( blockJob, blockData, progress, log)
{
//printf("SysBlock::SysBlock:\n");
   taskprogress = progress->tp[ data->getBlockNum()][0];
   taskprogress->state &= ~AFJOB::STATE_READY_MASK;
   taskprogress->starts_count = 0;
   taskprogress->errors_count = 0;
}

SysBlock::~SysBlock()
{
   for( std::list<SysCmd *>::iterator it = commands.begin(); it != commands.end(); it++) delete *it;
   for( std::list<SysTask*>::iterator it = systasks.begin(); it != systasks.end(); it++) delete *it;
}

void SysBlock::addCommand( SysCmd * syscmd)
{
   commands.push_back( syscmd);
   taskprogress->state |= AFJOB::STATE_READY_MASK;
}

bool SysBlock::isReady() const
{
   bool ready = true;

   if( getReadySysTask() == NULL )
   {
      if(( getNumCommands() < 1 ) || ( getNumSysTasks() >= af::Environment::getSysJobTasksMax() ))
      {
         taskprogress->state &= ~AFJOB::STATE_READY_MASK;
         ready = false;
      }
   }
   if( ready ) taskprogress->state |= AFJOB::STATE_READY_MASK;

   return ready;
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

   taskprogress->state |= AFJOB::STATE_RUNNING_MASK;
   taskprogress->starts_count++;

   if( monitoring ) tasks[0]->monitor( monitoring);
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
      AFCommon::QueueLogError("SysBlock::addTask: commands.size() == 0");
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
      std::string message = std::string("Can't find task number (max=") + af::itos(af::Environment::getSysJobTasksMax()) + ")";
      AFCommon::QueueLogError( std::string("SysBlock::addTask: %s") + message.c_str());
      appendJobLog( message);
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

void SysBlock::clearCommands()
{
//printf("SysBlock::clearCommands:\n");
   for( std::list<SysCmd *>::iterator it = commands.begin(); it != commands.end(); it++) delete *it;
   commands.clear();
   taskprogress->starts_count = 0;
   taskprogress->errors_count = 0;
}

void SysBlock::errorHostsAppend( int task, int hostId, RenderContainer * renders)
{
   RenderContainerIt rendersIt( renders);
   RenderAf* render = rendersIt.getRender( hostId);
   if( render == NULL ) return;
   if( Block::errorHostsAppend( render->getName())) appendJobLog( render->getName() + " - AVOIDING HOST !");
   SysTask * systask = getTask( task, "errorHostsAppend");
   if( systask) systask->errorHostsAppend( render->getName());
}

void SysBlock::getErrorHostsListString( std::string & str) const
{
   Block::getErrorHostsListString( str);
   for( std::list<SysTask*>::const_iterator it = systasks.begin(); it != systasks.end(); it++)
      (*it)->getErrorHostsListString( str);
}

void SysBlock::updateTaskState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("SysBlock::updateTaskState:\n");
   SysTask * systask = getTask( taskup.getNumTask(), "updateTaskState");
   if( systask == NULL ) return;
   bool errorHost = false;
   systask->updateState( taskup, renders, monitoring, errorHost);
   if( errorHost) errorHostsAppend( taskup.getNumTask(), taskup.getClientId(), renders);
}

bool SysBlock::refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("SysBlock::refresh:\n");
   bool blockProgress_changed = false;
   bool taskchanged = false;

   uint32_t blockstate_new = data->getState();
   uint32_t blockstate_old = blockstate_new;
   uint32_t taskstate_old = taskprogress->state;
   int tasksready_old = data->getProgressTasksReady();
   int tasksdone_old  = data->getProgressTasksDone();
   int taskserror_old = data->getProgressTasksError();
   int tasksready_new = 0;
   int tasksdone_new  = tasksdone_old;
   int taskserror_new = 0;

   blockstate_new &= ~AFJOB::STATE_RUNNING_MASK;
   taskprogress->state &= ~AFJOB::STATE_RUNNING_MASK;
   taskprogress->state &= ~AFJOB::STATE_READY_MASK;

   if( data->getRunningTasksNumber() > 0 ) blockstate_new |= AFJOB::STATE_RUNNING_MASK;
   data->setState( blockstate_new);

   for( std::list<SysTask*>::iterator it = systasks.begin(); it != systasks.end(); it++)
   {
      int errorHostId = -1;
      (*it)->refresh( currentTime, renders, monitoring, errorHostId);
      if( errorHostId != -1 ) errorHostsAppend( 0, errorHostId, renders);
      if((*it)->isRunning()) taskprogress->state |= AFJOB::STATE_RUNNING_MASK;
      if((*it)->isReady()  )
      {
         taskprogress->state |= AFJOB::STATE_READY_MASK;
         taskserror_new ++;
      }
   }

   tasksdone_new += deleteFinishedTasks( taskchanged);

   if( commands.size())
   {
      taskprogress->state |= AFJOB::STATE_READY_MASK;
      tasksready_new = commands.size();
   }

   if(  taskstate_old  != taskprogress->state ) taskchanged = true;
   if(( blockstate_old != blockstate_new ) ||
      ( tasksready_old != tasksready_new ) ||
      ( tasksdone_old  != tasksdone_new  ) ||
      ( taskserror_old != taskserror_new )  ) blockProgress_changed = true;

   if( taskchanged && monitoring) tasks[0]->monitor( monitoring);

   // For block in jobs list monitoring
   if( Block::refresh( currentTime, renders, monitoring))
   {
      // If block progress changed there, the function will add block in monitoring itself
      blockProgress_changed = true;
   }
   else
   {
      // Add block to monitoring if it was not, but has changes
      if( monitoring ) monitoring->addBlock( af::Msg::TBlocksProgress, data);
   }

   data->setProgressTasksReady( tasksready_new  );
   data->setProgressTasksDone(  tasksdone_new   );
   data->setProgressTasksError( taskserror_new  );

   return blockProgress_changed;
}

int SysBlock::deleteFinishedTasks( bool & taskProgressChanged)
{
   int done_tasks = 0;
   taskProgressChanged = false;
   for( std::list<SysTask*>::iterator it = systasks.begin(); it != systasks.end();)
   {
      if(( false == (*it)->isRunning()) && ((*it)->isError() || (*it)->isDone()))
      {
         if((*it)->isError())
         {
            taskprogress->errors_count++;
            taskProgressChanged = true;
         }
         else done_tasks++;
         delete *it;
         it = systasks.erase( it);
      }
      else it++;
   }
   return done_tasks;
}

SysTask * SysBlock::getTask( int tasknum, const char * errorMessage)
{
   for( std::list<SysTask*>::iterator it = systasks.begin(); it != systasks.end(); it++)
      if( tasknum == (*it)->getNumber()) return *it;
   if( errorMessage ) AFCommon::QueueLogError( std::string("SysJob::getTask: ") + errorMessage + ": Invalid task number = " + af::itos(tasknum));
   else               AFCommon::QueueLogError( std::string("SysJob::getTask: Invalid task number = ") + af::itos(tasknum));
   return NULL;
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

SysJob   * SysJob::sysjob        = NULL;
SysBlock * SysJob::block_cmdpost = NULL;
SysBlock * SysJob::block_wol     = NULL;

SysJob::SysJob( int flags):
   JobAf( AFJOB::SYSJOB_ID)
{
   sysjob = this;

   if( flags != New ) return;

   name              = AFJOB::SYSJOB_NAME;
   username          = AFJOB::SYSJOB_USERNAME;
   priority          = AFGENERAL::DEFAULT_PRIORITY;
   maxrunningtasks   = AFGENERAL::MAXRUNNINGTASKS;

   blocksnum = BlockLastIndex;
   blocksdata = new af::BlockData*[blocksnum];
   blocksdata[BlockPostCmdIndex] = new SysBlockData_CmdPost( BlockPostCmdIndex, id);
   blocksdata[BlockWOLIndex    ] = new SysBlockData_WOL(     BlockWOLIndex,     id);

   progress = new afsql::DBJobProgress( this);

   construct();
   fromdatabase = false;

   printf("System job constructed.\n");
}

SysJob::~SysJob()
{
}

void SysJob::dbDelete( std::list<std::string> * queries) const
{
   AFCommon::QueueLogError("Trying to delete system job from database.");
}

void SysJob::setZombie( RenderContainer * renders, MonitorContainer * monitoring)
{
   AFCommon::QueueLogError("Trying to make system job a zomibe.");
}

Block * SysJob::newBlock( int numBlock)
{
   switch( numBlock)
   {
   case BlockPostCmdIndex:
   {
      block_cmdpost = new SysBlock_CmdPost( this, blocksdata[numBlock], progress, &loglist);
      return block_cmdpost;
   }
   case BlockWOLIndex:
   {
      block_wol = new SysBlock_WOL( this, blocksdata[numBlock], progress, &loglist);
      return block_wol;
   }
   default:
      AFERRAR("SysJob::createBlock: Invalid block number = %d", numBlock)
   }
}

void SysJob::AddPostCommand( const std::string & Command, const std::string & WorkingDirectory, const std::string & UserName, const std::string & JobName)
{
   block_cmdpost->addCommand( new SysCmd( Command, WorkingDirectory, UserName, JobName));
}
void SysJob::AddWOLCommand(  const std::string & Command, const std::string & WorkingDirectory, const std::string & UserName, const std::string & JobName)
{
   block_wol->addCommand( new SysCmd( Command, WorkingDirectory, UserName, JobName));
}

bool SysJob::solve( RenderAf *render, MonitorContainer * monitoring)
{
//printf("SysJob::solve:\n");
   for( int b = 0; b < blocksnum; b++ )
      if(((SysBlock*)(blocks[b]))->isReady())
         return JobAf::solve( render, monitoring);

   return false;
}

void SysJob::updateTaskState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("SysJob::updateTaskState:\n");
//   JobAf::updateTaskState( taskup, renders, monitoring)

   if( taskup.getNumBlock() >= BlockLastIndex )
   {
      AFCommon::QueueLogError("SysJob::updateTaskState: Invalid block number = " + af::itos(taskup.getNumBlock()));
      return;
   }

   ((SysBlock*)(blocks[taskup.getNumBlock()]))->updateTaskState( taskup, renders, monitoring);
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
      case af::Msg::TBlockService:
      case af::Msg::TBlockParser:
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
      default:
         appendLog(std::string("Action \"") + af::Msg::TNAMES[type] + "\" is not available for the system job.");
   }

   return true;
}

void SysJob::restartTasks( const af::MCTasksPos &taskspos, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("SysJob::restartTasks:\n");
   for( int p = 0; p < taskspos.getCount(); p++)
   {
      int b = taskspos.getNumBlock(p);
      if( b >= blocksnum)
      {
         AFERRAR("SysJob::skipTasks: b >= blocksnum ( %d >= %d )", b, blocksnum)
         continue;
      }
      ((SysBlock*)(blocks[b]))->clearCommands();
   }
   JobAf::restartTasks( taskspos, renders, monitoring);
}

const std::string SysJob::getErrorHostsListString( int b, int t) const
{
   return "This is an empty dummy task in s system job block.\nGet job error hosts list to see its tasks error hosts.";
}

bool SysJob::getTaskStdOut( const af::MCTaskPos &taskpos, MsgAf *msg, std::string & filename, RenderContainer * renders)
{
   msg->setString("This is an empty dummy task in a system job block.\nError tasks output are stored in this task log.");
   return false;
}

void SysJob::appendJobLog( const std::string & message)
{
   sysjob->appendLog( message);
}

bool SysJob::isValid() const
{
   if( blocksnum != BlockLastIndex ) return false;
   return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   BLOCK DATA   //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SysBlockData::SysBlockData( int BlockNum, int JobId):
   afsql::DBBlockData( BlockNum, JobId)
{
//   initDefaults();
AFINFA("DBBlockData::DBBlockData: JobId=%d, BlockNum=%d", JobId, blocknum)

   capacity = af::Environment::getTaskDefaultCapacity();

   name = "system_commands";

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
