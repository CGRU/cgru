#include "taskrun.h"

#include "../libafanasy/blockdata.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/job.h"
#include "../libafanasy/taskexec.h"
#include "../libafanasy/msgclasses/mctaskpos.h"

#include "block.h"
#include "renderaf.h"
#include "rendercontainer.h"
#include "monitorcontainer.h"
#include "msgaf.h"
#include "task.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

TaskRun::TaskRun( Task * runningTask,
                  af::TaskExec* taskExec,
                  af::TaskProgress * taskProgress,
                  const Block * taskBlock,
                  RenderAf * render,
                  MonitorContainer * monitoring,
                  QStringList * taskLog,
                  int * runningtaskscounter
                  ):
   task( runningTask),
   block( taskBlock),
   exec( taskExec),
   progress( taskProgress),
   tasknum( 0),
   hostId( 0),
   counter( runningtaskscounter),
   logStingList( taskLog),
   stopTime( 0),
   zombie( false)
{
AFINFA("TaskRun::TaskRun: %s[%d][%d]:\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum);
   (*counter)++;

   progress->percent = -1;
   progress->frame = -1;
   progress->percentframe = -1;
   progress->hostname.clear();

   // Skip starting task if executable is not set (multihost task)
   if( exec == false) return;

   progress->state = AFJOB::STATE_RUNNING_MASK;
   progress->starts_count++;
   progress->time_start = time( NULL);
   progress->time_done = progress->time_start;
   tasknum = exec->getTaskNum();
   hostId = render->getId();
   progress->hostname = render->getName();
   render->setTask( exec, monitoring);
   task->monitor( monitoring );
   task->updateDatabase();
   log(QString("Starting on '%1'").arg(render->getName()));
}

TaskRun::~TaskRun()
{
AFINFA("TaskRun:: ~ TaskRun: %s[%d][%d]:\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum);
   if   ( *counter == 0) { AFERRAR("Tasks counter is negative ! (%d)\n", *counter); }
   else ( *counter )--;
   if( exec) delete exec;

   if( progress->state & AFJOB::STATE_DONE_MASK    ) return;
   if( progress->state & AFJOB::STATE_ERROR_MASK   ) return;
   if( progress->state & AFJOB::STATE_SKIPPED_MASK ) return;

   progress->state = AFJOB::STATE_READY_MASK;
}

void TaskRun::update( const af::MCTaskUp& taskup, RenderContainer * renders, MonitorContainer * monitoring, bool & errorHost)
{
   if( zombie )
   {
      AFERRAR("TaskRun::update: task is zombie: %d\n", tasknum);
      return;
   }
   if( taskup.getClientId() != hostId)
   {
      AFERRAR("TaskRun::update: taskup.getClientId() != hostId (%d != %d)\n", taskup.getClientId(), hostId);
      return;
   }
   if((progress->state & AFJOB::STATE_RUNNING_MASK) == false)
   {
      AFERRAR("TaskRun::update: %s[%d][%d] task is not running.\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum);
      return;
   }
   if( exec == NULL)
   {
      AFERRAR("TaskRun::update: %s[%d][%d] Task executable is NULL.\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum);
      return;
   }

   progress->time_done = time( NULL);

   QString message;

   switch ( taskup.getStatus())
   {
   case af::TaskExec::UPWarning:
      if( false == (progress->state & AFJOB::STATE_WARNING_MASK))
      {
         progress->state = progress->state | AFJOB::STATE_WARNING_MASK;
         task->updateDatabase();
      }
   case af::TaskExec::UPPercent:
//printf("TaskRun::update: case af::TaskExec::UPPercent:\n");
      progress->percent      = taskup.getPercent();
      progress->frame        = taskup.getFrame();
      progress->percentframe = taskup.getPercentFrame();
      task->monitor( monitoring );
   case af::TaskExec::UPStarted:
   case af::TaskExec::UPNULL:
   case af::TaskExec::UPNoTaskRunning:
   case af::TaskExec::UPNoJob:
   case af::TaskExec::UPLAST:
   {
      return;
   }
   case af::TaskExec::UPFinishedSuccess:
   {
      progress->state = progress->state | AFJOB::STATE_DONE_MASK;
      finish( "Finished success", renders, monitoring);
      break;
   }
   case af::TaskExec::UPFailedToStart: if( message.isEmpty()) message = "Failed to start.";
   case af::TaskExec::UPFinishedCrash: if( message.isEmpty()) message = "Finished crashed.";
   case af::TaskExec::UPFinishedParserError:
      if( message.isEmpty())
      {
         message = "Parser error.";
         progress->state = progress->state | AFJOB::STATE_PARSERERROR_MASK;
      }
   case af::TaskExec::UPFinishedParserBadResult:
      if( message.isEmpty())
      {
         message = "Parser bad result.";
         progress->state = progress->state | AFJOB::STATE_PARSERBADRESULT_MASK;
      }
   case af::TaskExec::UPFinishedError:
   {
      if( message.isEmpty()) message = "Finished error.";
      if( stopTime )
      {
         // Task was asked to be stopped before (skipped, restarted, ejected)
         finish("Stopped", renders, monitoring);
      }
      else
      {
         // Task stop was not planned, it is an error in any case
         progress->state = progress->state | AFJOB::STATE_ERROR_MASK;
         finish( message, renders, monitoring);
         progress->errors_count++;
         errorHost = true;
      }
      break;
   }
   case af::TaskExec::UPEject:
   {
      restart("Host owner ejected.", renders, monitoring);
      break;
   }
   case af::TaskExec::UPRenderDeregister:
   {
      finish("Finished - render deregistered.", renders, monitoring);
      break;
   }
   case af::TaskExec::UPRenderExit:
   {
      finish("Finished - render exit.", renders, monitoring);
      break;
   }
   case af::TaskExec::UPRenderZombie:
   {
      finish("Finished - render zombie.", renders, monitoring);
      break;
   }
   default:
   {
      AFERRAR("TaskRun::update: Unknown task update status = %d\n", taskup.getStatus());
      return;
   }
   }
}

bool TaskRun::refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring, int & errorHostId)
{
   if( exec == NULL)
   {
      AFERRAR("TaskRun::refresh: %s[%d][%d] Task executable is NULL.\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum);
      return false;
   }
//printf("TaskRun::refresh: %s[%d][%d]\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum);
   if( zombie ) return false;
   bool changed = false;

   // Tasks with running time > maximum set to errors: // If it TasksMaxRunTime > 0 ( 0 is "infinite" )
   if((block->data->getTasksMaxRunTime() != 0) && (currentTime - progress->time_start > block->data->getTasksMaxRunTime()))
   {
      stop("Task maximum run time reached", renders, monitoring);
      errorHostId = hostId;
   }

   // Tasks update timeout check:
   if(( stopTime == 0) && ( currentTime > progress->time_done + af::Environment::getTaskUpdateTimeout()))
   {
      stop("Task update timeout", renders, monitoring);
      errorHostId = hostId;
   }

   // Tasks stop timeout check:
   if( stopTime &&( currentTime - stopTime > AFJOB::TASK_STOP_TIMEOUT ))
   {
      finish("Task stop timeout", renders, monitoring);
      if( changed == false) changed = true;
      errorHostId = hostId;
   }

   return changed;
}

void TaskRun::stop( const QString & message, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("TaskRun::stop: %s[%d][%d] HostID=%d\n\t%s\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum, hostId, message.toUtf8().data());
   if( zombie ) return;
   if( stopTime ) return;
   if( exec == NULL)
   {
      AFERRAR("TaskRun::stop: %s[%d][%d] Task executable is NULL.\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum);
      return;
   }
   stopTime = time( NULL);

   if( hostId != 0 )
   {
      RenderContainerIt rendersIt( renders);
      RenderAf * render = rendersIt.getRender( hostId);
      if( render ) render->stopTask( exec);
   }

   log( message);
}

void TaskRun::finish( const QString & message, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("TaskRun::finish: %s[%d][%d] HostID=%d\n\t%s\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum, hostId, message.toUtf8().data());
   if( zombie ) return;

   stopTime = 0;
   progress->state = progress->state & (~AFJOB::STATE_RUNNING_MASK);

   if((hostId != 0) && exec)
   {
      RenderContainerIt rendersIt( renders);
      RenderAf * render = rendersIt.getRender( hostId);
      if( render ) render->taskFinished( exec, monitoring);
   }

   task->monitor( monitoring );
   task->updateDatabase();

   log( message);
   zombie = true;
}

void TaskRun::restart( const QString & message, RenderContainer * renders, MonitorContainer * monitoring)
{
   if( zombie ) return;
   stop( message+" (is running)", renders, monitoring);
}

void TaskRun::skip( const QString & message, RenderContainer * renders, MonitorContainer * monitoring)
{
   if( zombie ) return;
   progress->state = progress->state | AFJOB::STATE_SKIPPED_MASK;
   progress->state = progress->state | AFJOB::STATE_DONE_MASK;
   stop( message+" (is running)", renders, monitoring);
}

void TaskRun::listen( af::MCListenAddress & mclisten, RenderContainer * renders)
{
   if( zombie ) return;
   if( hostId == 0 ) return;
   if( exec == NULL)
   {
      AFERRAR("TaskRun::listen: %s[%d][%d] Task executable is NULL.\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum);
      return;
   }
   RenderContainerIt rendersIt( renders);
   RenderAf * render = rendersIt.getRender( hostId);
   if( render != NULL) render->sendOutput( mclisten, block->job->getId(), block->data->getBlockNum(), tasknum);
}

bool TaskRun::getOutput( int startcount, MsgAf *msg, RenderContainer * renders) const
{
   if( exec == NULL)
   {
      QString str("TaskRun::getOutput: Task is not started.");
      msg->setString( str);
      return false;
   }
   if( hostId > 0 )
   {
      RenderContainerIt rendersIt( renders);
      RenderAf * render = rendersIt.getRender( hostId);
      if( render != NULL )
      {
         af::MCTaskPos taskpos( block->job->getId(), block->data->getBlockNum(), tasknum);
         msg->set( af::Msg::TTaskOutputRequest, &taskpos);
         msg->setAddress( render);
         return true;
      }
      else
      {
         QString str("TaskRun::getOutput: render == NULL");
         msg->setString( str);
         return false;
      }
   }
   else
   {
      QString str("TaskRun::getOutput: render_id == 0");
      msg->setString( str);
      return false;
   }
  return true;
}

int TaskRun::calcWeight() const
{
   int weight = sizeof( TaskRun);
   if( exec) weight += exec->calcWeight();
   return weight;
}
