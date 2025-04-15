#include "block.h"

#include "../include/afanasy.h"

#include "../libafanasy/jobprogress.h"

#include "action.h"
#include "afcommon.h"
#include "jobaf.h"
#include "monitorcontainer.h"
#include "rendercontainer.h"
#include "renderaf.h"
#include "task.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

Block::Block( JobAf * blockJob, af::BlockData * blockData, af::JobProgress * progress):
   m_job( blockJob),
   m_data( blockData),
   m_tasks( NULL),
   m_user( NULL),
   m_jobprogress( progress),
   m_initialized( false)
{
   if (!allocateTasks())
      return;
   constructDependBlocks();
   m_initialized = true;
}

Block::~Block()
{
   if( m_tasks)
   {
	  for( int t = 0; t < m_data->getTasksNum(); t++) if( m_tasks[t]) delete m_tasks[t];
	  delete [] m_tasks;
   }
}

bool Block::storeTasks()
{
	if( m_data->isNumeric()) return true;

	std::ostringstream str;
	str << "{\n";
	m_data->jsonWriteTasks( str);
	str << "\n}";

	return AFCommon::writeFile( str, getStoreTasksFileName());
}

bool Block::readStoredTasks()
{
	if( m_data->isNumeric()) return true;

	int size;
	char * data = af::fileRead( getStoreTasksFileName(), &size);
	if( data == NULL ) return false;

	rapidjson::Document document;
	char * res = af::jsonParseData( document, data, size);
	if( res == NULL )
	{
		delete [] data;
		return false;
	}

	m_data->jsonReadTasks( document);

	delete [] data;
	delete [] res;

	return true;
}

const std::string Block::getStoreTasksFileName() const
{
	return m_job->getStoreDir() + AFGENERAL::PATH_SEPARATOR + "block" + af::itos( m_data->getBlockNum()) + "_tasks.json";
}

void Block::appendJobLog(const std::string & i_info, bool i_store)
{
	af::Log log;
	log.type = "jobs";
	log.object = m_job->getName();
	log.info = "Block[\"" + m_data->getName() + "\"]: " + i_info;
	m_job->appendLog(log, i_store);
}

void Block::v_errorHostsAppend( int task, int hostId, RenderContainer * renders)
{
   if( task >= m_data->getTasksNum())
   {
	  AFERRAR("Block::errorHostsAppend: task >= tasksnum (%d>=%d)", task, m_data->getTasksNum())
      return;
   }
   RenderContainerIt rendersIt( renders);
   RenderAf* render = rendersIt.getRender( hostId);
   if( render == NULL ) return;
   if( v_errorHostsAppend( render->getName())) appendJobLog( render->getName()+ " - AVOIDING HOST !", false);
   m_tasks[task]->errorHostsAppend( render->getName());
}

bool Block::v_errorHostsAppend( const std::string & hostname)
{
   std::list<std::string>::iterator hIt = m_errorHosts.begin();
   std::list<int>::iterator cIt = m_errorHostsCounts.begin();
   std::list<time_t>::iterator tIt = m_errorHostsTime.begin();
   for( ; hIt != m_errorHosts.end(); hIt++, tIt++, cIt++ )
      if( *hIt == hostname )
      {
         (*cIt)++;
         *tIt = time(NULL);
         if( *cIt >= getErrorsAvoidHost()) return true;
         return false;
      }

   m_errorHosts.push_back( hostname);
   m_errorHostsCounts.push_back( 1);
   m_errorHostsTime.push_back( time(NULL));
   return false;
}

bool Block::avoidHostsCheck( const std::string & hostname) const
{
   if( getErrorsAvoidHost() < 1 ) return false;
   std::list<std::string>::const_iterator hIt = m_errorHosts.begin();
   std::list<int>::const_iterator cIt = m_errorHostsCounts.begin();
   std::list<time_t>::const_iterator tIt = m_errorHostsTime.begin();
   for( ; hIt != m_errorHosts.end(); hIt++, tIt++, cIt++ )
      if( *hIt == hostname )
      {
         if( *cIt >= getErrorsAvoidHost() )
         {
            return true;
         }
         else
         {
            return false;
         }
      }

   return false;
}

void Block::v_getErrorHostsList( std::list<std::string> & o_list) const
{
	o_list.push_back( std::string("Block['") + m_data->getName() + "'] error hosts:");
   std::list<std::string>::const_iterator hIt = m_errorHosts.begin();
   std::list<int>::const_iterator cIt = m_errorHostsCounts.begin();
   std::list<time_t>::const_iterator tIt = m_errorHostsTime.begin();
   for( ; hIt != m_errorHosts.end(); hIt++, tIt++, cIt++ )
   {
		std::string str = *hIt + ": " + af::itos( *cIt) + " at " + af::time2str( *tIt);
      if(( getErrorsAvoidHost() > 0 ) && ( *cIt >= getErrorsAvoidHost())) str += " - ! AVOIDING !";
		o_list.push_back( str);
   }

	for( int t = 0; t < m_data->getTasksNum(); t++)
		m_tasks[t]->getErrorHostsList( o_list);
}

void Block::v_errorHostsReset()
{
   m_errorHosts.clear();
   m_errorHostsCounts.clear();
   m_errorHostsTime.clear();
   for( int t = 0; t < m_data->getTasksNum(); t++) m_tasks[t]->errorHostsReset();
}

bool Block::v_startTask( af::TaskExec * taskexec, RenderAf * render, MonitorContainer * monitoring)
{
   // Set variable capacity to maximum value:
   if( m_data->canVarCapacity() && (taskexec->getCapacity() > 0))
   {
      int cap_coeff = render->findCapacityFree() / taskexec->getCapacity();
	  if( cap_coeff < m_data->getCapCoeffMin())
      {
		 AFERRAR("Block::startTask: cap_coeff < data->getCapCoeffMin(%d<%d)", cap_coeff, m_data->getCapCoeffMin())
      }
      else
      {
		 if(( m_data->getCapCoeffMax() > 0 ) && (cap_coeff > m_data->getCapCoeffMax())) cap_coeff = m_data->getCapCoeffMax();
         taskexec->setCapCoeff( cap_coeff);
      }
   }

   m_tasks[taskexec->getTaskNum()]->v_start( taskexec, render, monitoring);

   return true;
}

void Block::reconnectTask(af::TaskExec *i_taskexec, RenderAf & i_render, MonitorContainer * i_monitoring)
{
	Task * task = m_tasks[i_taskexec->getTaskNum()];
	task->reconnect( i_taskexec, &i_render, i_monitoring);
}

bool Block::canRunOn( RenderAf * render)
{
	// Check max running tasks on the same host:
	if (m_data->getMaxRunTasksPerHost() == 0)
		return false;
	if ((m_data->getMaxRunTasksPerHost() > 0) && (getRenderCount(render) >= m_data->getMaxRunTasksPerHost()))
		return false;

	// Check available capacity:
	if (false == render->hasCapacity(m_data->getCapMinResult()))
		return false;

	// render services:
	if (false == render->canRunService(m_data->getService()))
		return false;

	// Check Tickets:
	if (false == render->hasTickets(m_data->getTickets()))
		return false;

	// check maximum hosts:
	if ((m_data->getMaxRunningTasks() >= 0) && (m_data->getRunningTasksNumber() >= m_data->getMaxRunningTasks()))
		return false;

	// Check block avoid hosts list:
	if (avoidHostsCheck(render->getName()))
		return false;

	// Check needed memory:
	if (m_data->getNeedMemory() > render->getHostRes().mem_free_mb)
		return false;

	// Check needed GPU memory:
	if (m_data->getNeedGPUMemMb() > (render->getHostRes().gpu_mem_total_mb - render->getHostRes().gpu_mem_used_mb))
		return false;

	// Check needed CPU frequency:
	if (m_data->getNeedCPUFreqMHz() > render->getHostRes().cpu_mhz)
		return false;

	// Check needed CPU cores:
	if (m_data->getNeedCPUCores() > render->getHostRes().cpu_num)
		return false;

	// Check needed CPU frequency * cores:
	if (m_data->getNeedCPUFreqCores() > (render->getHostRes().cpu_num * render->getHostRes().cpu_mhz))
		return false;

	// Check needed hdd:
	if (m_data->getNeedHDD() > render->getHostRes().hdd_free_gb)
		return false;

	// check hosts mask:
	if (false == m_data->checkHostsMask(render->getName()))
		return false;

	// check exclude hosts mask:
	if (false == m_data->checkHostsMaskExclude(render->getName()))
		return false;

	// Check needed power:
	if (false == m_data->checkNeedPower(render->findPower()))
		return false;

	// Check needed properties:
	if (false == m_data->checkNeedProperties(render->findProperties()))
		return false;

	return true;
}

void Block::addSolveCounts(MonitorContainer * i_monitoring, af::TaskExec * i_exec, RenderAf * i_render)
{
	addRenderCount(i_render);

	m_data->addSolveCounts(i_exec, i_render);

	m_job->addSolveCounts(i_monitoring, i_exec, i_render);
}

void Block::remSolveCounts(MonitorContainer * i_monitoring, af::TaskExec * i_exec, RenderAf * i_render)
{
	remRenderCount(i_render);

	m_data->remSolveCounts(i_exec, i_render);

	m_job->remSolveCounts(i_monitoring, i_exec, i_render);
}

void Block::addRenderCount(RenderAf * i_render)
{
	std::list<RenderAf*>::iterator rit = m_renders_ptrs.begin();
	std::list<int>::iterator cit = m_renders_counts.begin();
	for ( ; rit != m_renders_ptrs.end(); rit++, cit++)
		if (i_render == *rit )
		{
			(*cit)++;
			return;
		}
	m_renders_ptrs.push_back(i_render);
	m_renders_counts.push_back(1);
}

int Block::getRenderCount(RenderAf * i_render) const
{
	std::list<RenderAf*>::const_iterator rit = m_renders_ptrs.begin();
	std::list<int>::const_iterator cit = m_renders_counts.begin();
	for ( ; rit != m_renders_ptrs.end(); rit++, cit++)
		if (i_render == *rit)
			return *cit;
	return 0;
}

void Block::remRenderCount(RenderAf * i_render)
{
	std::list<RenderAf*>::iterator rit = m_renders_ptrs.begin();
	std::list<int>::iterator cit = m_renders_counts.begin();
	for( ; rit != m_renders_ptrs.end(); rit++, cit++)
		if (i_render == *rit)
		{
			if (*cit > 1)
				(*cit)--;
			else
			{
				m_renders_ptrs.erase(rit);
				m_renders_counts.erase(cit);
			}
			return;
		}
}

bool Block::v_refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring)
{
   if( m_user == NULL)
   {
      AFERROR("Block::refresh: User is not set.")
      return false;
   }

   // refresh tasks
   for( int t = 0; t < m_data->getTasksNum(); t++)
   {
      int errorHostId = -1;
	  m_tasks[t]->v_refresh( currentTime, renders, monitoring, errorHostId);
      if( errorHostId != -1 ) v_errorHostsAppend( t, errorHostId, renders);
   }

   // For block progress monitoring in jobs list and in tasks list
   bool blockProgress_changed = false;

   // store old state to know if monitoring and database udate needed
   uint32_t old_block_state = m_data->getState();

   // forgive error hosts
   if(( false == m_errorHosts.empty() ) && ( getErrorsForgiveTime() > 0 ))
   {
	  std::list<std::string>::iterator hIt = m_errorHosts.begin();
	  std::list<int>::iterator cIt = m_errorHostsCounts.begin();
	  std::list<time_t>::iterator tIt = m_errorHostsTime.begin();
	  while( hIt != m_errorHosts.end() )
         if( currentTime - *tIt > getErrorsForgiveTime())
         {
            appendJobLog( std::string("Forgived error host \"") + *hIt + "\" since " + af::time2str(*tIt) + ".", false);
			hIt = m_errorHosts.erase( hIt);
			cIt = m_errorHostsCounts.erase( cIt);
			tIt = m_errorHostsTime.erase( tIt);
         }
         else
         {
            hIt++;
            cIt++;
            tIt++;
         }
    }


   // calculate number of error and avoid hosts for monitoring
   {
      int avoidhostsnum = 0;
	  int errorhostsnum = m_errorHosts.size();
      if(( errorhostsnum != 0 ) && ( getErrorsAvoidHost() > 0 ))
		 for( std::list<int>::const_iterator cIt = m_errorHostsCounts.begin(); cIt != m_errorHostsCounts.end(); cIt++)
            if( *cIt >= getErrorsAvoidHost())
               avoidhostsnum++;

	  if(( m_data->getProgressErrorHostsNum() != errorhostsnum ) ||
		 ( m_data->getProgressAvoidHostsNum() != avoidhostsnum ) )
      {
         blockProgress_changed = true;
      }

	  m_data->setProgressErrorHostsNum( errorhostsnum);
	  m_data->setProgressAvoidHostsNum( avoidhostsnum);
   }

	// No need to update progress in sys job block, it will be updated in virtual function customly
	// ( if it will be updated here, it will always return that it changes )
	if (m_job->getId() != AFJOB::SYSJOB_ID)
	{
		if (false == (m_data->getState() & AFJOB::STATE_DONE_MASK))
			if (checkTasksDependStatus(monitoring))
				blockProgress_changed = true;

		// Update block tasks progress and status
		if (m_data->updateProgress(m_jobprogress))
			blockProgress_changed = true;

		// Update progress bars for GUIs:
		m_data->updateBars(m_jobprogress);
	}

	if (old_block_state != m_data->getState())
		blockProgress_changed = true;

   // update block monitoring and database if needed
   if( blockProgress_changed && monitoring )
		monitoring->addBlock( af::Msg::TBlocksProgress, m_data);

   return blockProgress_changed;
}

bool Block::checkBlockDependStatus(MonitorContainer * i_monitoring)
{
	bool was_depend = m_data->getState() & AFJOB::STATE_WAITDEP_MASK;
	bool now_depend = false;

	if( m_dependBlocks.size())
		for( std::list<int>::const_iterator bIt = m_dependBlocks.begin(); bIt != m_dependBlocks.end(); bIt++)
		{
			if (m_job->getBlockData(*bIt)->getState() & AFJOB::STATE_DONE_MASK)
				continue;

			now_depend = true;
			break;
		}

	if( now_depend != was_depend )
	{
		m_data->setStateDependent( now_depend);

		if (false == now_depend)
		{
			// If the block just stop to depend, its status should be recalculated.
			// Or it will loose depend state and will not get ready state evet if it has ready tasks.
			m_data->updateProgress(m_jobprogress);
		}

		if( i_monitoring )
			i_monitoring->addBlock( af::Msg::TBlocksProgress, m_data);

		return true;
	}

	return false;
}

bool Block::resetTasksDependStatus(MonitorContainer * i_monitoring)
{
	bool some_task_state_changed = false;

	for (int task = 0; task < m_data->getTasksNum(); task++)
	{
		int64_t state = m_jobprogress->tp[m_data->getBlockNum()][task]->state;

		if (false == (state & AFJOB::STATE_WAITDEP_MASK))
			continue;

		state = state & (~AFJOB::STATE_WAITDEP_MASK);
		state = state | AFJOB::STATE_READY_MASK;

		m_tasks[task]->m_dependent.clear();
		m_tasks[task]->m_depend_on.clear();

		m_jobprogress->tp[m_data->getBlockNum()][task]->state = state;
		m_tasks[task]->v_monitor(i_monitoring);
		some_task_state_changed = true;
	}

	return some_task_state_changed;
}

bool Block::checkTasksDependStatus(MonitorContainer * i_monitoring)
{
	if (m_job->getBlocksNum() < 2)
		return false;

	if (m_dependTasksBlocks.size() == 0)
		return false;

	bool some_task_state_changed = false;

	for (int task = 0; task < m_data->getTasksNum(); task++)
	{
		int64_t state = m_jobprogress->tp[m_data->getBlockNum()][task]->state;

		if (state & AFJOB::STATE_WAITDEP_MASK)
		{
			state = state & (~AFJOB::STATE_WAITDEP_MASK);
			state = state | AFJOB::STATE_READY_MASK;
		}

		if (false == (state & AFJOB::STATE_READY_MASK))
			continue;

		for (int t = 0; t < m_tasks[task]->m_depend_on.size(); t++)
		{
			Task * dep_on_task = m_tasks[task]->m_depend_on[t];

			if (dep_on_task->isDone())
				continue;

			// Check subframe depend, is depend task is running:
			if (dep_on_task->getBlock()->m_data->isDependSubTask() && dep_on_task->isRunning())
			{
				long long firstdependframe, lastdependframe;
				m_data->genNumbers(firstdependframe, lastdependframe, task);
				if (m_data->isNumeric() && dep_on_task->getBlock()->m_data->isNotNumeric())
				{
					firstdependframe -= m_data->getFrameFirst();
					lastdependframe  -= m_data->getFrameFirst();
				}
				else if (m_data->isNotNumeric() && dep_on_task->getBlock()->m_data->isNumeric())
				{
					firstdependframe += dep_on_task->getBlock()->m_data->getFrameFirst();
					lastdependframe  += dep_on_task->getBlock()->m_data->getFrameFirst();
				}

				if (dep_on_task->getBlock()->m_data->getFramePerTask() < 0)
					lastdependframe++; // For several frames in task

				long long f_start_dep, f_end_dep;
				dep_on_task->getBlock()->m_data->genNumbers(f_start_dep, f_end_dep, dep_on_task->getNumber());
				long long frame_run = f_start_dep + dep_on_task->getProgressFrame();
				if (frame_run > lastdependframe)
					continue;
			}

			state = state | AFJOB::STATE_WAITDEP_MASK;
			state = state & (~AFJOB::STATE_READY_MASK);

			break;
		}

		if (state != m_jobprogress->tp[m_data->getBlockNum()][task]->state)
		{
			m_jobprogress->tp[m_data->getBlockNum()][task]->state = state;
			m_tasks[task]->v_monitor(i_monitoring);
			some_task_state_changed = true;
		}
	}

	return some_task_state_changed;
}

bool Block::action( Action & i_action)
{
	uint32_t blockchanged_type = 0;
	bool job_changed = false;

	const JSON & operation = (*i_action.data)["operation"];
	if( operation.IsObject())
	{
		std::string type;
		af::jr_string("type", type, operation);
		i_action.log.appendType(type);

		if (type == "reset_error_hosts")
		{
			v_errorHostsReset();
			if (blockchanged_type < af::Msg::TBlocksProperties)
				blockchanged_type = af::Msg::TBlocksProperties;
			job_changed = true;
		}
		else if (type == "skip")
		{
			//tasksOperation("Tasks skip by " + i_action.author, i_action, operation, ~AFJOB::STATE_DONE_MASK, AFJOB::STATE_SKIPPED_MASK | AFJOB::STATE_DONE_MASK);
			tasksOperation(i_action, operation, ~AFJOB::STATE_DONE_MASK, AFJOB::STATE_SKIPPED_MASK | AFJOB::STATE_DONE_MASK);
		}
		else if (type == "suspend")
		{
			//tasksOperation("Tasks suspend by " + i_action.author, i_action, operation,
			tasksOperation(i_action, operation,
					AFJOB::STATE_READY_MASK | AFJOB::STATE_RUNNING_MASK | AFJOB::STATE_SKIPPED_MASK, AFJOB::STATE_SUSPENDED_MASK);
		}
		else if (type == "continue")
		{
			//tasksOperation("Tasks continue by " + i_action.author, i_action, operation, AFJOB::STATE_SUSPENDED_MASK, AFJOB::STATE_READY_MASK);
			tasksOperation(i_action, operation, AFJOB::STATE_SUSPENDED_MASK, AFJOB::STATE_READY_MASK);
		}
		else if (type == "done")
		{
			//tasksOperation("Tasks set done by " + i_action.author, i_action, operation, ~AFJOB::STATE_DONE_MASK, AFJOB::STATE_DONE_MASK);
			tasksOperation(i_action, operation, ~AFJOB::STATE_DONE_MASK, AFJOB::STATE_DONE_MASK);
		}
		else if (type == "restart")
		{
			//tasksOperation("Tasks restart by " + i_action.author, i_action, operation, 0 /*with any task*/, AFJOB::STATE_READY_MASK);
			tasksOperation(i_action, operation, 0 /*with any task*/, AFJOB::STATE_READY_MASK);
		}
		else if (type == "restart_running")
		{
			//tasksOperation("Restart running tasks by " + i_action.author, i_action, operation, AFJOB::STATE_RUNNING_MASK, 0 /*does not set any state*/);
			tasksOperation(i_action, operation, AFJOB::STATE_RUNNING_MASK, 0 /*does not set any state*/);
		}
		else if (type == "restart_skipped")
		{
			//tasksOperation("Restart skipped tasks by " + i_action.author, i_action, operation, AFJOB::STATE_SKIPPED_MASK, AFJOB::STATE_READY_MASK);
			tasksOperation(i_action, operation, AFJOB::STATE_SKIPPED_MASK, AFJOB::STATE_READY_MASK);
		}
		else if (type == "restart_done")
		{
			//tasksOperation("Restart done tasks by " + i_action.author, i_action, operation, AFJOB::STATE_DONE_MASK, AFJOB::STATE_READY_MASK);
			tasksOperation(i_action, operation, AFJOB::STATE_DONE_MASK, AFJOB::STATE_READY_MASK);
		}
		else if (type == "restart_errors")
		{
			//tasksOperation("Restart error tasks by " + i_action.author, i_action, operation, AFJOB::STATE_ERROR_MASK, AFJOB::STATE_READY_MASK);
			tasksOperation(i_action, operation, AFJOB::STATE_ERROR_MASK, AFJOB::STATE_READY_MASK);
		}
		else if (type == "trynext")
		{
			if (tryTasksNext(i_action, operation))
			{
				blockchanged_type = af::Msg::TBlocksProgress;
				job_changed = true;
			}
		}
		else if (type == "append_tasks")
		{
			if (appendTasks(i_action, operation))
			{
				// Add a log line so that store() is called at the job level (a bit hacky)
				i_action.log.info += "\nBlock['" + m_data->getName() + "']: Append tasks";
				return true;
			}
			return false;
		}
		else if (type == "tickets")
		{
			if (editTickets(i_action, operation))
			{
				blockchanged_type = af::Msg::TBlocksProperties;
				job_changed = true;
			}
		}
		else
		{
			i_action.answerError("Unknown operation: " + type);
			return false;
		}

		//appendJobLog("Operation \"" + type + "\" by " + i_action.author);
	}
	else
	{
		const JSON & params = (*i_action.data)["params"];
		if( params.IsObject())
		{
			std::string changes;
			m_data->jsonRead( params, &changes);
			i_action.log.appendType("params");

			if( changes.empty())
				return false;

			i_action.log.info += "\bBlock['" + m_data->getName() + "']: " + changes;

			blockchanged_type = af::Msg::TBlocksProperties;
			job_changed = true;
			constructDependBlocks();

			if (m_dependTasksBlocks.size() == 0)
				resetTasksDependStatus(i_action.monitors);
			else
				constructDependTasks();
		}
	}

	if( blockchanged_type )
	{
//AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)m_data);
		i_action.monitors->addBlock( af::Msg::TBlocksProperties, m_data);
	}

	return job_changed;
}

bool Block::editTickets(Action & i_action, const JSON & operation)
{
	std::string name;
	if (false == af::jr_string("name", name, operation))
	{
		i_action.answerError("Ticket 'name' string is not specified.");
		return false;
	}

	int32_t count;
	if (false == af::jr_int32("count", count, operation))
	{
		i_action.answerError("Ticket 'count' integer is not specified.");
		return false;
	}

	m_data->editTicket(name, count);

	return true;
}

void Block::tasksOperation(Action & i_action, const JSON & i_operation, uint32_t i_with_state, uint32_t i_set_state)
{
	std::vector<int32_t> tasks_vec;
	af::jr_int32vec("task_ids", tasks_vec, i_operation);

	int length = m_data->getTasksNum();
	if( tasks_vec.size())
		length = tasks_vec.size();

	for( int i = 0; i < length; i++)
	{
		int t = i;
		if( tasks_vec.size())
		{
			t = tasks_vec[i];
			if(( t >= m_data->getTasksNum()) || ( t < 0 ))
			{
				i_action.answerError("Invalid operation task numer = " + af::itos(t));
				return;
			}
		}

		m_tasks[t]->operation(i_action.log, i_action.renders, i_action.monitors, i_with_state, i_set_state);

		m_job->forceRefresh();
	}
}

bool Block::tryTasksNext(Action & i_action, const JSON & i_operation)
{
	if (m_job->isMaintenanceFlag())
	{
		i_action.answerError("Maintenance job can't try tasks next.");
		return false;
	}

	std::vector<int32_t> tasks_vec;
	af::jr_int32vec("task_ids", tasks_vec, i_operation);
	std::string mode;
	af::jr_string("mode", mode, i_operation);

	if (tasks_vec.size() == 0)
	{
		i_action.answerError("'task_ids' array is not specified.");
		return false;
	}

	bool append;
	if (mode == "append")
		append = true;
	else if (mode == "remove")
		append = false;
	else
	{
		i_action.answerError("Invalid 'mode' = '" + mode + "'.");
		return false;
	}

	bool success = false;
	for (int i = 0; i < tasks_vec.size(); i++)
	{
		int t = tasks_vec[i];

		if ((t >= m_data->getTasksNum()) || ( t < 0 ))
		{
			i_action.answerError("Invalid operation task numer = " + af::itos(t));
			break;
		}

		if (m_tasks[t]->tryNext(append, i_action.monitors))
		{
			m_job->tryTaskNext(append, m_data->getBlockNum(), t);
			success = true;
		}
	}

	if (success)
	{
		i_action.answerInfo("Tasks to try next processed.");
	}
	else
	{
		i_action.answerError("Unable to find valid tasks.");
	}

	return success;
}

bool Block::allocateTasks(int alreadyAllocated)
{
	Task **old_tasks = m_tasks;

	if (m_data->getTasksNum() <= 0)
	{
		AF_ERR << "Block '" + m_data->getName() + "' has a zero tasks number.";
		return false;
	}
	m_tasks = new Task *[m_data->getTasksNum()];
	if (m_tasks == NULL)
	{
		AFERROR("Blocl::Block: Can't allocate memory for tasks.")
		if (NULL != old_tasks)
			delete [] old_tasks;
		return false;
	}
	for (int t = 0; t < m_data->getTasksNum(); t++)
		m_tasks[t] = t < alreadyAllocated ? old_tasks[t] : NULL;
	if (NULL != old_tasks)
		delete [] old_tasks;
	for (int t = alreadyAllocated; t < m_data->getTasksNum(); t++)
	{
		m_tasks[t] = new Task(this, m_jobprogress->tp[m_data->getBlockNum()][t], t);
		if (m_tasks == NULL)
		{
			AFERRAR("Blocl::Block: Can't allocate memory for task %d of %d.", t, m_data->getTasksNum())
			return false;
		}
	}
	return true;
}

void Block::constructDependBlocks()
{
	if( m_job->getBlocksNum() <= 1 )
    {
        // There is only one block, no one other to denend on
        return;
    }

	m_dependBlocks.clear();
	m_dependTasksBlocks.clear();

	if( m_data->hasDependMask())
    {
		for( int bd = 0; bd < m_job->getBlocksNum(); bd++)
        {
            // skip if it is the same block
			if( bd == m_data->getBlockNum() ) continue;

            // store block if name match mask
			if (m_data->checkDependMask(m_job->getBlockData(bd)->getName()))
            m_dependBlocks.push_back( bd);
        }
    }

	if( m_data->hasTasksDependMask())
    {
		for( int bd = 0; bd < m_job->getBlocksNum(); bd++)
        {
            // skip if it is the same block
			if( bd == m_data->getBlockNum() ) continue;

            // store block if name match mask
			if (m_data->checkTasksDependMask(m_job->getBlockData(bd)->getName()))
            m_dependTasksBlocks.push_back( bd);
        }
    }
}

void Block::constructDependTasks()
{
	if (m_job->getBlocksNum() < 2)
		return;

	if (m_dependTasksBlocks.size() == 0)
		return;

	for (int task = 0; task < m_data->getTasksNum(); task++)
	{
		m_tasks[task]->m_dependent.clear();
		m_tasks[task]->m_depend_on.clear();

		for (int & b : m_dependTasksBlocks)
		{
			long long firstdependframe, lastdependframe;
			m_data->genNumbers(firstdependframe, lastdependframe, task);
			if (m_data->isNumeric() && m_job->getBlockData(b)->isNotNumeric())
			{
				firstdependframe -= m_data->getFrameFirst();
				lastdependframe  -= m_data->getFrameFirst();
			}
			else if (m_data->isNotNumeric() && m_job->getBlockData(b)->isNumeric())
			{
				firstdependframe += m_job->getBlockData(b)->getFrameFirst();
				lastdependframe  += m_job->getBlockData(b)->getFrameFirst();
			}

			if (m_job->getBlockData(b)->getFramePerTask() < 0)
				lastdependframe++; // For several frames in task

			int firstdependtask, lastdependtask;
			bool inValidRange;
			firstdependtask = m_job->getBlockData(b)->calcTaskNumber(firstdependframe, inValidRange);
			lastdependtask  = m_job->getBlockData(b)->calcTaskNumber( lastdependframe, inValidRange);
			if (inValidRange)
				if (m_job->getBlockData(b)->getFramePerTask() < 0)
					lastdependtask--;

			for (int t = firstdependtask; t <= lastdependtask; t++)
			{
				Task * depTask = m_job->getBlock(b)->m_tasks[t];
				m_tasks[task]->m_depend_on.push_back(depTask);
				depTask->m_dependent.push_back(m_tasks[task]);
//				af::addUniqueToVect(m_tasks[task]->m_depend_on, t);
//				af::addUniqueToVect(m_tasks[t]->m_dependent, task);
//if (m_jobprogress->tp[b][t]->state & (AFJOB::STATE_DONE_MASK | AFJOB::STATE_SKIPPED_MASK))
			}
		}
	}
}

bool Block::tasksDependsOn( int block)
{
    for( std::list<int>::const_iterator it = m_dependTasksBlocks.begin(); it != m_dependTasksBlocks.end(); it++)
        if( *it == block )
            return true;
    return false;
}

int Block::calcWeight() const
{
   int weight = sizeof( Block) + m_data->calcWeight();
   for( int t = 0; t < m_data->getTasksNum(); t++) weight += m_tasks[t]->calcWeight();
   return weight;
}

int Block::logsWeight() const
{
   int weight = 0;
   for( int t = 0; t < m_data->getTasksNum(); t++) weight += m_tasks[t]->logsWeight();
   return weight;
}

int Block::blackListWeight() const
{
   int weight = sizeof(int) * m_errorHostsCounts.size();
   weight += af::weigh( m_errorHosts);
   for( int t = 0; t < m_data->getTasksNum(); t++) weight += m_tasks[t]->blackListWeight();
   return weight;
}

void Block::checkStatesOnAppend()
{
	for (int t = 0; t < m_data->getTasksNum(); t++)
	{
		uint32_t taskstate = m_jobprogress->tp[m_data->getBlockNum()][t]->state;

		if (taskstate == 0)
		{
			if (m_data->isSuspendingNewTasks())
				taskstate = AFJOB::STATE_SUSPENDED_MASK;
			else
				taskstate = AFJOB::STATE_READY_MASK;
			m_jobprogress->tp[m_data->getBlockNum()][t]->state = taskstate;
		}
	}

	constructDependBlocks();
	if (m_dependTasksBlocks.size())
		constructDependTasks();
}

bool Block::appendTasks(Action & i_action, const JSON & i_operation)
{
	if (m_job->getId() == AFJOB::SYSJOB_ID)
	{
		i_action.answerError("Appending system job is not allowed.");
		return false;
	}

	if (m_data->isNumeric())
	{
		i_action.answerError("Appending tasks to numeric block is not allowed.");
		return false;
	}

	const JSON &tasks = i_operation["tasks"];
	if (!tasks.IsArray())
	{
		i_action.answerError("Operation requires tasks array.");
		return false;
	}

	// Allocate new tasks
	int old_tasks_num = m_data->getTasksNum();
	m_data->jsonReadAndAppendTasks(i_operation);
	m_jobprogress->appendTasks(m_data->getBlockNum(), m_data->getTasksNum() - old_tasks_num);
	allocateTasks(old_tasks_num); // allocate only new tasks

	// Store tasks
	storeTasks();

	// Set new tasks ready
	checkStatesOnAppend();

	// Emit an event for monitors (afwatch ListTasks)
	i_action.monitors->addBlock(af::Msg::TBlocks, m_data);

	// Return new tasks ids:
	std::string answer = "{\"task_ids\":[";
	for (int i = old_tasks_num; i < m_data->getTasksNum(); i++)
	{
		if (i != old_tasks_num)
			answer += ",";
		answer += af::itos(i);
	}
	answer += "]}";
	i_action.answerObject(answer);

	return true;
}
