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
                  MonitorContainer * monitoring
                  ):
   m_task( runningTask),
   m_block( taskBlock),
   m_exec( taskExec),
   m_progress( taskProgress),
   m_tasknum( 0),
   m_hostId( 0),
   m_stopTime( 0),
   m_zombie( false)
{
	AF_DEBUG << "TaskRun::TaskRun: " << m_block->m_job->getName() << "[" << m_block->m_data->getBlockNum() << "][" << m_tasknum << "]:";

   m_progress->percent = -1;
   m_progress->frame = -1;
   m_progress->percentframe = -1;
   m_progress->hostname.clear();
	m_progress->activity.clear();

	// Skip starting task if executable is not set (multihost task)
	if( m_exec == NULL) return;

	// Let block increase renders counters.
	// Needed to max run tasks per host limit.
	// This block function also adds counts on its job.
	m_block->addSolveCounts(monitoring, m_exec, render);
	//^Multihost task will increase capacity counter itself.

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

void TaskRun::update(const af::MCTaskUp& taskup, RenderContainer * renders, MonitorContainer * monitoring, bool & o_error_host)
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
			m_progress->last_percent_change = m_progress->time_done;
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

		// Check minimum running time:
		// It should be checked only if task
		// UPFinishedSuccess (finished itself with a good exit status), or
		// UPFinishedFailedPost (finished itself with a good exit status, but post task command was failed).
		// And this check definitely should be skipped in UPSkip (when user skipped running task).
		// Also it should be skipped on UPFinishedParserSuccess, as in parser we already made a decision that render was good.
		if ((m_stopTime == 0) && (m_block->m_data->getTaskMinRunTime() > 0))
		{
			if ((taskup.getStatus() == af::TaskExec::UPFinishedSuccess) ||
				(taskup.getStatus() == af::TaskExec::UPFinishedFailedPost))
			{
				int run_time = m_progress->time_done - m_progress->time_start;
				if (run_time < m_block->m_data->getTaskMinRunTime())
				{
					m_progress->state = m_progress->state | AFJOB::STATE_ERROR_MASK;
					m_progress->errors_count++;
					message = "Task running time (" + af::itos(run_time) + "s) is less than a minimum (" + af::itos(m_block->m_data->getTaskMinRunTime()) + "s)";
					finish(message, renders, monitoring);
					o_error_host = true;
					break;
				}
			}
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
         o_error_host = true;
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

	// Tasks stop timeout check:
	if( m_stopTime && ( currentTime - m_stopTime > af::Environment::getTaskStopTimeout()))
	{
		finish("Task stop timeout.", renders, monitoring);
		if( changed == false) changed = true;
		errorHostId = m_hostId;
	}

	// Next checks not needed it the task is stopping:
	if (m_stopTime)
		return changed;
	
	// Max running time check:
	if ((m_block->m_data->getTaskMaxRunTime() != 0) && // ( If TasksMaxRunTime == 0 it is "infinite" )
		(currentTime - m_progress->time_start > m_block->m_data->getTaskMaxRunTime()))
	{
		m_progress->state = m_progress->state | AFJOB::STATE_ERROR_MASK;
		m_progress->errors_count++;
		stop("Task maximum run time reached.", renders, monitoring);
		errorHostId = m_hostId;
	}

	// Tasks update timeout check:
	if (currentTime > (m_progress->time_done + af::Environment::getTaskUpdateTimeout()))
	{
		//printf("Task update timeout: %d > %d+%d\n", currentTime, m_progress->time_done, af::Environment::getTaskUpdateTimeout());
		stop("Task update timeout.", renders, monitoring);
		errorHostId = m_hostId;
	}

	// Tasks progress change timeout:
	int timeout = m_block->m_data->getTaskProgressChangeTimeout();
	int no_progress_for = currentTime - m_progress->last_percent_change;
	if ((timeout > 0 ) && (no_progress_for > timeout))
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
			m_block->remSolveCounts(monitoring, m_exec, render);
		}

	  	// Write database for statistics:
		AFCommon::DBAddTask( m_exec, m_progress, m_block->m_job, render);

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
