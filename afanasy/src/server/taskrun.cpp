#include "taskrun.h"

#include "../libafanasy/blockdata.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/job.h"
#include "../libafanasy/msgqueue.h"
#include "../libafanasy/msgclasses/mctaskpos.h"

#include "afcommon.h"
#include "block.h"
#include "jobaf.h"
#include "renderaf.h"
#include "rendercontainer.h"
#include "monitorcontainer.h"
#include "task.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

std::string TaskRun::ms_no_name = "no_exec_name";
  
TaskRun::TaskRun( Task * runningTask,
                  af::TaskExec* taskExec,
                  af::TaskProgress * taskProgress,
                  Block * taskBlock,
                  RenderAf * render,
                  MonitorContainer * monitoring,
                  int32_t * i_running_tasks_counter,
                  int64_t * i_running_capacity_counter
                  ):
   m_task( runningTask),
   m_block( taskBlock),
   m_exec( taskExec),
   m_progress( taskProgress),
   m_tasknum( 0),
   m_hostId( 0),
	m_running_tasks_counter( i_running_tasks_counter),
	m_running_capacity_counter( i_running_capacity_counter),
   m_stopTime( 0),
   m_zombie( false)
{
	AF_DEBUG << "TaskRun::TaskRun: " << m_block->m_job->getName() << "[" << m_block->m_data->getBlockNum() << "][" << m_tasknum << "]:";
	(*m_running_tasks_counter)++;

   m_progress->percent = -1;
   m_progress->frame = -1;
   m_progress->percentframe = -1;
   m_progress->hostname.clear();
	m_progress->activity.clear();

	// Let block increase renders counters.
	// Needed to max run tasks per host limit.
	// This block function also adds counts on its job.
	m_block->addRenderCounts( render);

	// Skip starting task if executable is not set (multihost task)
	if( m_exec == NULL) return;

	(*m_running_capacity_counter) += m_exec->getCapResult();
	// - Multihost task will increase capacity counter itself.

   m_progress->state = AFJOB::STATE_RUNNING_MASK;
   m_progress->starts_count++;
   m_progress->time_start = time( NULL);
   m_progress->last_percent_change = time( NULL);
   m_progress->time_done = m_progress->time_start;
   m_tasknum = m_exec->getTaskNum();
   m_hostId = render->getId();
   m_progress->hostname = render->getName();
	m_exec->setProgress( m_progress);
   render->setTask( m_exec, monitoring);
   m_task->v_monitor( monitoring );
   m_task->v_store();
   m_task->v_appendLog( std::string("SESSION #") + af::itos( m_progress->starts_count) + ": Starting on \"" + render->getName() + "\"");
}

TaskRun::~TaskRun()
{
AFINFA("TaskRun:: ~ TaskRun: %s[%d][%d]:", m_block->m_job->getName().c_str(), m_block->m_data->getBlockNum(), m_tasknum)

   if( m_progress->state & AFJOB::STATE_DONE_MASK    ) return;
   if( m_progress->state & AFJOB::STATE_ERROR_MASK   ) return;
   if( m_progress->state & AFJOB::STATE_SKIPPED_MASK ) return;

   m_progress->state = AFJOB::STATE_READY_MASK;
}

void TaskRun::update( const af::MCTaskUp& taskup, RenderContainer * renders, MonitorContainer * monitoring, bool & errorHost)
{
	if( m_zombie )
	{
		AF_ERR << "task is zombie: " << m_tasknum;
		return;
	}
	if( taskup.getClientId() != m_hostId)
	{
		AF_ERR << "taskup.getClientId() != hostId (" << taskup.getClientId() << " != " << m_hostId << ")";
		return;
	}
	if((m_progress->state & AFJOB::STATE_RUNNING_MASK) == false)
	{
		AF_ERR << m_block->m_job->getName() << "[" << m_block->m_data->getBlockNum() << "][" << m_tasknum << "] task is not running.";
		return;
	}
	if( NULL == m_exec)
	{
		AF_ERR << m_block->m_job->getName() << "[" << m_block->m_data->getBlockNum() << "][" << m_tasknum << "] Task executable is NULL.";
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
			m_task->v_store();
		}
	case af::TaskExec::UPPercent:
	{
		//printf("TaskRun::update: case af::TaskExec::UPPercent:\n");
		int new_percent = taskup.getPercent();
		if (new_percent != m_progress->percent)
			m_progress->last_percent_change = time( NULL);
		m_progress->percent      = new_percent;
		m_progress->frame        = taskup.getFrame();
		m_progress->percentframe = taskup.getPercentFrame();
		if( taskup.getActivity().size() > 0 ) m_progress->activity = taskup.getActivity();
		m_task->v_monitor( monitoring );
	}
	case af::TaskExec::UPStarted:
	case af::TaskExec::UPNULL:
	case af::TaskExec::UPNoTaskRunning:
	case af::TaskExec::UPNoJob:
	case af::TaskExec::UPLAST:
	{
		return;
	}
	case af::TaskExec::UPFinishedParserSuccess:
		if( message.empty())
		{
			message = "Parser finished success.";
			m_progress->state = m_progress->state | AFJOB::STATE_PARSERSUCCESS_MASK;
		}
	case af::TaskExec::UPFinishedFailedPost:
		if( message.empty())
		{
			message = "Finished, but post failed.";
			m_progress->state = m_progress->state | AFJOB::STATE_FAILEDPOST_MASK;
		}
	case af::TaskExec::UPSkip:
		if( message.empty())
		{
			message = "Skipped by service on render.";
			m_progress->state = m_progress->state | AFJOB::STATE_SKIPPED_MASK;
		}
	case af::TaskExec::UPFinishedSuccess:
	{
		if( message.empty())
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
   case af::TaskExec::UPBadRenderedFiles:
      if( message.size() == 0)
      {
         message = "Bad rendered files.";
         m_progress->state = m_progress->state | AFJOB::STATE_BADRENDEREDFILES_MASK;
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
      AFERRAR("TaskRun::update: Unknown task update status = %d", taskup.getStatus())
   }
}

bool TaskRun::refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring, int & errorHostId)
{
	if( m_zombie ) return false;
	
	if( NULL == m_exec)
	{
		AF_ERR << "TaskRun::refresh: " << m_block->m_job->getName() << "["
		       << m_block->m_data->getBlockNum() << "][" << m_tasknum
		       << "] Task executable is NULL.";
		return false;
	}
	//printf("TaskRun::refresh: %s[%d][%d]\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum);
	bool changed = false;

	// Max running time check:
	if(( m_block->m_data->getTasksMaxRunTime() != 0) && // ( If TasksMaxRunTime == 0 it is "infinite" )
		( m_stopTime == 0 ) && // It can be already reachedd before and task is already stopping
		( currentTime - m_progress->time_start > m_block->m_data->getTasksMaxRunTime()))
	{
		m_progress->state = m_progress->state | AFJOB::STATE_ERROR_MASK;
		m_progress->errors_count++;
		stop("Task maximum run time reached.", renders, monitoring);
		errorHostId = m_hostId;
	}

	// Tasks update timeout check:
	if(( m_stopTime == 0) && ( currentTime > m_progress->time_done + af::Environment::getTaskUpdateTimeout()))
	{
		//printf("Task update timeout: %d > %d+%d\n", currentTime, m_progress->time_done, af::Environment::getTaskUpdateTimeout());
		stop("Task update timeout.", renders, monitoring);
		errorHostId = m_hostId;
	}

	// Tasks stop timeout check:
	if( m_stopTime && ( currentTime - m_stopTime > af::Environment::getTaskStopTimeout()))
	{
		finish("Task stop timeout.", renders, monitoring);
		if( changed == false) changed = true;
		errorHostId = m_hostId;
	}
	
	// Tasks progress change timeout
	int timeout = m_block->m_data->getTaskProgressChangeTimeout();
	int no_progress_for = currentTime - m_progress->last_percent_change;
	if (timeout > 0 && no_progress_for > timeout)
	{
		m_progress->errors_count++;
		stop("Task run time without progress reached (no progress for " + af::itos(no_progress_for) + "s).", renders, monitoring);
		errorHostId = m_hostId;
	}

	return changed;
}

void TaskRun::stop( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("TaskRun::stop: %s[%d][%d] HostID=%d: %s\n", m_block->m_job->getName().c_str(), m_block->m_data->getBlockNum(), m_tasknum, m_hostId, message.c_str());
   if( m_zombie ) return;
   if( m_stopTime ) return;
   if( NULL == m_exec)
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
			m_block->remRenderCounts( render);
		}

	  	// Write database for statistics:
		AFCommon::DBAddTask( m_exec, m_progress, m_block->m_job, render);

		// Decrement counters:
		if( *m_running_tasks_counter <= 0)
			AF_ERR << "Tasks counter is zero or negative: " << *m_running_tasks_counter;
		else
			( *m_running_tasks_counter )--;

		if( *m_running_capacity_counter <= 0)
			AF_ERR << "Tasks capacity counter is zero or negative: " << *m_running_capacity_counter;
		else
			( *m_running_capacity_counter ) -= m_exec->getCapResult();

		// Delete task executable:
		delete m_exec;
		m_exec = NULL;
   }

   m_task->v_monitor( monitoring );
   m_task->v_store();

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

int TaskRun::v_getRunningRenderID( std::string & o_error) const
{
	if( NULL == m_exec)
	{
		o_error = "Not started.";
		return 0;
	}

	if( m_hostId > 0 )
	{
		return m_hostId;
	}

	o_error = "Zero render ID.";
	return 0;
}

int TaskRun::calcWeight() const
{
   int weight = sizeof( TaskRun);
   if( m_exec) weight += m_exec->calcWeight();
   return weight;
}
