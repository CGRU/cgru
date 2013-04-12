#include "taskrun.h"

#include "../libafanasy/blockdata.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/job.h"
#include "../libafanasy/taskexec.h"
#include "../libafanasy/msgqueue.h"
#include "../libafanasy/msgclasses/mctaskpos.h"

#include "block.h"
#include "jobaf.h"
#include "renderaf.h"
#include "rendercontainer.h"
#include "monitorcontainer.h"
#include "task.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

TaskRun::TaskRun( Task * runningTask,
                  af::TaskExec* taskExec,
                  af::TaskProgress * taskProgress,
                  Block * taskBlock,
                  RenderAf * render,
                  MonitorContainer * monitoring,
                  int * runningtaskscounter
                  ):
   m_task( runningTask),
   m_block( taskBlock),
   m_exec( taskExec),
   m_progress( taskProgress),
   m_tasknum( 0),
   m_hostId( 0),
   m_counter( runningtaskscounter),
   m_stopTime( 0),
   m_zombie( false)
{
AFINFA("TaskRun::TaskRun: %s[%d][%d]:", m_block->m_job->getName().c_str(), m_block->m_data->getBlockNum(), m_tasknum)
   (*m_counter)++;

   m_progress->percent = -1;
   m_progress->frame = -1;
   m_progress->percentframe = -1;
   m_progress->hostname.clear();
	m_progress->activity.clear();

	// Skip starting task if executable is not set (multihost task)
	if( m_exec == NULL) return;

   m_progress->state = AFJOB::STATE_RUNNING_MASK;
   m_progress->starts_count++;
   m_progress->time_start = time( NULL);
   m_progress->time_done = m_progress->time_start;
   m_tasknum = m_exec->getTaskNum();
   m_hostId = render->getId();
   m_progress->hostname = render->getName();
	m_exec->setProgress( m_progress);
   render->setTask( m_exec, monitoring);
   m_task->v_monitor( monitoring );
   m_task->v_updateDatabase();
   m_task->v_appendLog( std::string("SESSION #") + af::itos( m_progress->starts_count) + ": Starting on \"" + render->getName() + "\"");
}

TaskRun::~TaskRun()
{
AFINFA("TaskRun:: ~ TaskRun: %s[%d][%d]:", m_block->m_job->getName().c_str(), m_block->m_data->getBlockNum(), m_tasknum)
   if   ( *m_counter == 0) AFERRAR("Tasks counter is negative ! (%d)", *m_counter)
   else ( *m_counter )--;
   if( m_exec) delete m_exec;

   if( m_progress->state & AFJOB::STATE_DONE_MASK    ) return;
   if( m_progress->state & AFJOB::STATE_ERROR_MASK   ) return;
   if( m_progress->state & AFJOB::STATE_SKIPPED_MASK ) return;

   m_progress->state = AFJOB::STATE_READY_MASK;
}

void TaskRun::update( const af::MCTaskUp& taskup, RenderContainer * renders, MonitorContainer * monitoring, bool & errorHost)
{
   if( m_zombie )
   {
      AFERRAR("TaskRun::update: task is zombie: %d", m_tasknum)
      return;
   }
   if( taskup.getClientId() != m_hostId)
   {
      AFERRAR("TaskRun::update: taskup.getClientId() != hostId (%d != %d)", taskup.getClientId(), m_hostId)
      return;
   }
   if((m_progress->state & AFJOB::STATE_RUNNING_MASK) == false)
   {
      AFERRAR("TaskRun::update: %s[%d][%d] task is not running.", m_block->m_job->getName().c_str(), m_block->m_data->getBlockNum(), m_tasknum)
      return;
   }
   if( m_exec == NULL)
   {
      AFERRAR("TaskRun::update: %s[%d][%d] Task executable is NULL.", m_block->m_job->getName().c_str(), m_block->m_data->getBlockNum(), m_tasknum)
      return;
   }

   m_progress->time_done = time( NULL);

   std::string message;

   switch ( taskup.getStatus())
   {
   case af::TaskExec::UPWarning:
      if( false == (m_progress->state & AFJOB::STATE_WARNING_MASK))
      {
         m_progress->state = m_progress->state | AFJOB::STATE_WARNING_MASK;
         m_task->v_updateDatabase();
      }
   case af::TaskExec::UPPercent:
//printf("TaskRun::update: case af::TaskExec::UPPercent:\n");
      m_progress->percent      = taskup.getPercent();
      m_progress->frame        = taskup.getFrame();
      m_progress->percentframe = taskup.getPercentFrame();
		if( taskup.getActivity().size() > 0 ) m_progress->activity = taskup.getActivity();
      m_task->v_monitor( monitoring );
   case af::TaskExec::UPStarted:
   case af::TaskExec::UPNULL:
   case af::TaskExec::UPNoTaskRunning:
   case af::TaskExec::UPNoJob:
   case af::TaskExec::UPLAST:
   {
      return;
   }
	case af::TaskExec::UPFinishedParserSuccess:
		if( message.size() == 0)
		{
			message = "Parser finished success.";
			m_progress->state = m_progress->state | AFJOB::STATE_PARSERSUCCESS_MASK;
		}
	case af::TaskExec::UPFinishedSuccess:
	{
		if( message.size() == 0)
		{
			message = "Finished success.";
		}
		m_progress->state = m_progress->state | AFJOB::STATE_DONE_MASK;
		finish( message, renders, monitoring);
		break;
	}
   case af::TaskExec::UPFailedToStart: if( message.size() == 0) message = "Failed to start.";
   case af::TaskExec::UPFinishedKilled: if( message.size() == 0) message = "Killed.";
   case af::TaskExec::UPFinishedParserError:
      if( message.size() == 0)
      {
         message = "Parser error.";
         m_progress->state = m_progress->state | AFJOB::STATE_PARSERERROR_MASK;
      }
   case af::TaskExec::UPFinishedParserBadResult:
      if( message.size() == 0)
      {
         message = "Parser bad result.";
         m_progress->state = m_progress->state | AFJOB::STATE_PARSERBADRESULT_MASK;
      }
   case af::TaskExec::UPFinishedError:
   {
      if( message.size() == 0) message = "Finished error.";
      if( m_stopTime )
      {
         // Task was asked to be stopped before (skipped, restarted, ejected)
         finish("Stopped.", renders, monitoring);
      }
      else
      {
         // Task stop was not planned, it is an error in any case
         m_progress->state = m_progress->state | AFJOB::STATE_ERROR_MASK;
         // Increment number of errors before Finish provedure
         m_progress->errors_count++;
         // In Finish procedure task updates it's progress database (and write there new number of errors)
         finish( message, renders, monitoring);
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
      AFERRAR("TaskRun::update: Unknown task update status = %d", taskup.getStatus())
      return;
   }
   }
}

bool TaskRun::refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring, int & errorHostId)
{
   if( m_exec == NULL)
   {
      AFERRAR("TaskRun::refresh: %s[%d][%d] Task executable is NULL.", m_block->m_job->getName().c_str(), m_block->m_data->getBlockNum(), m_tasknum)
      return false;
   }
//printf("TaskRun::refresh: %s[%d][%d]\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum);
   if( m_zombie ) return false;
   bool changed = false;

   // Tasks with running time > maximum set to errors: // If it TasksMaxRunTime > 0 ( 0 is "infinite" )
   if((m_block->m_data->getTasksMaxRunTime() != 0) && (currentTime - m_progress->time_start > m_block->m_data->getTasksMaxRunTime()))
   {
      stop("Task maximum run time reached.", renders, monitoring);
      errorHostId = m_hostId;
   }

   // Tasks update timeout check:
   if(( m_stopTime == 0) && ( currentTime > m_progress->time_done + af::Environment::getTaskUpdateTimeout()))
   {
      stop("Task update timeout.", renders, monitoring);
      errorHostId = m_hostId;
   }

   // Tasks stop timeout check:
   if( m_stopTime &&( currentTime - m_stopTime > AFJOB::TASK_STOP_TIMEOUT ))
   {
      finish("Task stop timeout.", renders, monitoring);
      if( changed == false) changed = true;
      errorHostId = m_hostId;
   }

   return changed;
}

void TaskRun::stop( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("TaskRun::stop: %s[%d][%d] HostID=%d\n\t%s\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum, hostId, message.toUtf8().data());
   if( m_zombie ) return;
   if( m_stopTime ) return;
   if( m_exec == NULL)
   {
      AFERRAR("TaskRun::stop: %s[%d][%d] Task executable is NULL.", m_block->m_job->getName().c_str(), m_block->m_data->getBlockNum(), m_tasknum)
      return;
   }
   m_stopTime = time( NULL);

   if( m_hostId != 0 )
   {
      RenderContainerIt rendersIt( renders);
      RenderAf * render = rendersIt.getRender( m_hostId);
      if( render ) render->stopTask( m_exec);
   }

   m_task->v_appendLog( message);
}

void TaskRun::finish( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("TaskRun::finish: %s[%d][%d] HostID=%d\n\t%s\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum, hostId, message.toUtf8().data());
   if( m_zombie ) return;

   m_stopTime = 0;
   m_progress->state = m_progress->state & (~AFJOB::STATE_RUNNING_MASK);
	m_progress->activity.clear();

   if((m_hostId != 0) && m_exec)
   {
      RenderContainerIt rendersIt( renders);
      RenderAf * render = rendersIt.getRender( m_hostId);
      if( render )
      {
         render->taskFinished( m_exec, monitoring);
         m_block->taskFinished( m_exec, render, monitoring);
      }
   }

   m_task->v_monitor( monitoring );
   m_task->v_updateDatabase();

   m_task->v_appendLog( message);
   m_zombie = true;
}

void TaskRun::restart( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
{
   if( m_zombie ) return;
   stop( message+" Is running.", renders, monitoring);
}

void TaskRun::skip( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
{
   if( m_zombie ) return;
   m_progress->state = m_progress->state | AFJOB::STATE_SKIPPED_MASK;
   m_progress->state = m_progress->state | AFJOB::STATE_DONE_MASK;
   stop( message+" Is running.", renders, monitoring);
}

void TaskRun::listen( af::MCListenAddress & mclisten, RenderContainer * renders)
{
   if( m_zombie ) return;
   if( m_hostId == 0 ) return;
   if( m_exec == NULL)
   {
      AFERRAR("TaskRun::listen: %s[%d][%d] Task executable is NULL.", m_block->m_job->getName().c_str(), m_block->m_data->getBlockNum(), m_tasknum)
      return;
   }
printf("Listening running task:"); mclisten.v_stdOut();
   RenderContainerIt rendersIt( renders);
   RenderAf * render = rendersIt.getRender( m_hostId);
   if( render != NULL) render->sendOutput( mclisten, m_block->m_job->getId(), m_block->m_data->getBlockNum(), m_tasknum);
}

af::Msg * TaskRun::v_getOutput( int i_startcount, RenderContainer * i_renders, std::string & o_error) const
{
	if( m_exec == NULL)
	{
		o_error = "Not started.";
		return NULL;
	}
	if( m_hostId > 0 )
	{
		RenderContainerIt rendersIt( i_renders);
		RenderAf * render = rendersIt.getRender( m_hostId);
		if( render != NULL )
		{
			af::MCTaskPos taskpos( m_block->m_job->getId(), m_block->m_data->getBlockNum(), m_tasknum);
			af::Msg * msg = new af::Msg( af::Msg::TTaskOutputRequest, &taskpos);
			msg->setAddress( render);
			return msg;
		}
		else
		{
			o_error = "TaskRun::getOutput: render is NULL.";
			return NULL;
		}
	}

	o_error = "TaskRun::getOutput: Zero render_id.";
	return NULL;
}

int TaskRun::calcWeight() const
{
   int weight = sizeof( TaskRun);
   if( m_exec) weight += m_exec->calcWeight();
   return weight;
}
