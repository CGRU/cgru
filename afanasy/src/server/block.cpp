#include "block.h"

#include "../libafanasy/jobprogress.h"

#include "../libafsql/dbattr.h"
#include "../libafsql/dbblockdata.h"

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

Block::Block( JobAf * blockJob, af::BlockData * blockData, af::JobProgress * progress):
   m_job( blockJob),
   m_data( blockData),
   m_tasks( NULL),
   m_user( NULL),
   m_jobprogress( progress),
   m_initialized( false)
{
   m_tasks = new Task*[ m_data->getTasksNum()];
   if( m_tasks == NULL )
   {
      AFERROR("Blocl::Block: Can't allocate memory for tasks.")
      return;
   }
   for( int t = 0; t < m_data->getTasksNum(); t++) m_tasks[t] = NULL;
   for( int t = 0; t < m_data->getTasksNum(); t++)
   {
	  m_tasks[t] = new Task( this, progress->tp[ m_data->getBlockNum()][t], t);
	  if( m_tasks == NULL )
      {
		 AFERRAR("Blocl::Block: Can't allocate memory for task %d of %d.", t, m_data->getTasksNum())
         return;
      }
   }
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

void Block::appendJobLog( const std::string & message)
{
   m_job->appendLog("B[\"" + m_data->getName() + "\"]: " + message);
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
   if( v_errorHostsAppend( render->getName())) appendJobLog( render->getName()+ " - AVOIDING HOST !");
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

void Block::v_startTask( af::TaskExec * taskexec, RenderAf * render, MonitorContainer * monitoring)
{
   // Store block name in task executable:
   taskexec->setBlockName( m_data->getName());

   // Set variable capacity to maximum value:
   if( m_data->canVarCapacity() && (taskexec->getCapacity() > 0))
   {
      int cap_coeff = render->getCapacityFree() / taskexec->getCapacity();
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

   // Store render pointer:
   addRenderCounts( render);

   m_tasks[taskexec->getTaskNum()]->v_start( taskexec, m_data->getRunningTasksCounter(), render, monitoring);
}

void Block::taskFinished( af::TaskExec * taskexec, RenderAf * render, MonitorContainer * monitoring)
{
   remRenderCounts( render);
}

bool Block::canRunOn( RenderAf * render)
{
   // check max running tasks on the same host:
   if(  m_data->getMaxRunTasksPerHost() == 0 ) return false;
   if(( m_data->getMaxRunTasksPerHost()  > 0 ) && ( getRenderCounts(render) >= m_data->getMaxRunTasksPerHost() )) return false;
   // check available capacity:
   if( false == render->hasCapacity( m_data->getCapMinResult())) return false;
   // render services:
   if( false == render->canRunService( m_data->getService())) return false;
   // check maximum hosts:
   if(( m_data->getMaxRunningTasks() >= 0 ) && ( m_data->getRunningTasksNumber() >= m_data->getMaxRunningTasks() )) return false;
   // Check block avoid hosts list:
   if( avoidHostsCheck( render->getName()) ) return false;
   // Check task avoid hosts list:
   if( m_data->getNeedMemory() > render->getHostRes().mem_free_mb ) return false;
   // Check needed hdd:
   if( m_data->getNeedHDD()    > render->getHostRes().hdd_free_gb ) return false;
   // Check needed power:
   if( m_data->getNeedPower()  > render->getHost().m_power       ) return false;

   // check hosts mask:
   if( false == m_data->checkHostsMask( render->getName())) return false;
   // check exclude hosts mask:
   if( false == m_data->checkHostsMaskExclude( render->getName())) return false;
   // Check needed properties:
   if( false == m_data->checkNeedProperties( render->getHost().m_properties)) return false;

   return true;
}

void Block::addRenderCounts( RenderAf * render)
{
   m_job->addRenderCounts( render);
   std::list<RenderAf*>::iterator rit = m_renders_ptrs.begin();
   std::list<int>::iterator cit = m_renders_counts.begin();
   for( ; rit != m_renders_ptrs.end(); rit++, cit++)
      if( render == *rit )
      {
         (*cit)++;
         return;
      }
   m_renders_ptrs.push_back( render);
   m_renders_counts.push_back( 1);
}

int Block::getRenderCounts( RenderAf * render) const
{
   std::list<RenderAf*>::const_iterator rit = m_renders_ptrs.begin();
   std::list<int>::const_iterator cit = m_renders_counts.begin();
   for( ; rit != m_renders_ptrs.end(); rit++, cit++)
      if( render == *rit ) return *cit;
   return 0;
}

void Block::remRenderCounts( RenderAf * render)
{
   m_job->remRenderCounts( render);
   std::list<RenderAf*>::iterator rit = m_renders_ptrs.begin();
   std::list<int>::iterator cit = m_renders_counts.begin();
   for( ; rit != m_renders_ptrs.end(); rit++, cit++)
      if( render == *rit )
      {
         if( *cit > 1 )
            (*cit)--;
         else
         {
			m_renders_ptrs.erase( rit);
			m_renders_counts.erase( cit);
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
            appendJobLog( std::string("Forgived error host \"") + *hIt + "\" since " + af::time2str(*tIt) + ".");
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
   if( m_job->getId() != AFJOB::SYSJOB_ID )
   // Update block tasks progress and bars
	  if( m_data->updateProgress( m_jobprogress)) blockProgress_changed = true;

   //
   // Blocksdata depend check
   m_data->setStateDependent( false);
   if( m_dependBlocks.size())
      for( std::list<int>::const_iterator bIt = m_dependBlocks.begin(); bIt != m_dependBlocks.end(); bIt++)
      {
		 if( m_job->getBlock(*bIt)->getState() & AFJOB::STATE_DONE_MASK) continue;
		 m_data->setStateDependent( true);
         break;
      }
      //printf("Block::refresh: checking '%s': %s\n", data->getName().toUtf8().data(), data->state & AFJOB::STATE_READY_MASK ? "READY" : "NOT ready");

   if( old_block_state != m_data->getState()) blockProgress_changed = true;

   // update block monitoring and database if needed
   if( blockProgress_changed && monitoring )
   {
	  if( monitoring ) monitoring->addBlock( af::Msg::TBlocksProgress, m_data);

      // No need to update state in database, state is calculated attribute
      // AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_state);
   }

   return blockProgress_changed;
}

bool Block::action( Action & i_action)
{
	uint32_t blockchanged_type = 0;
	bool job_progress_changed = false;

	const JSON & operation = (*i_action.data)["operation"];
	if( operation.IsObject())
	{
		std::string type;
		af::jr_string("type", type, operation);
		if( type == "reset_error_hosts")
		{
			v_errorHostsReset();
			if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
			job_progress_changed = true;
		}
		else if(( type == "restart") || ( type == "skip") || ("restart_running"))
		{
			skipRestartTasks( i_action, operation, type);
		}
		else
		{
			appendJobLog("Unknown block operation \"" + type + "\" by " + i_action.author);
			return false;
		}

		appendJobLog("Operation \"" + type + "\" by " + i_action.author);
	}
	else
	{
		const JSON & params = (*i_action.data)["params"];
		if( params.IsObject())
		{
			std::string changes;
			m_data->jsonRead( params, &changes);

			if( changes.empty())
				return false;

			i_action.log += "\nBlock['" + m_data->getName() + "']:" + changes;

			blockchanged_type = af::Msg::TBlocksProperties;
			job_progress_changed = true;
		}
	}

	if( blockchanged_type )
	{
		AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)m_data);
		i_action.monitors->addBlock( af::Msg::TBlocksProperties, m_data);
	}

	return job_progress_changed;
}

void Block::skipRestartTasks( const Action & i_action, const JSON & i_operation, const std::string & i_type)
{
	bool skip = false;
	bool only_running = false;
	if( i_type == "skip")
		skip = true;
	else if( i_type == "restart_running")
		only_running = true;

	std::string message;
	if( skip ) message = "Skip request by ";
	else       message = "Restart request by ";
	message += i_action.author;

	std::vector<int32_t> tasks_vec;
	af::jr_int32vec("task_ids", tasks_vec, i_operation);

	int length = m_data->getTasksNum();
	if( tasks_vec.size())
		length = tasks_vec.size();

	AFCommon::QueueDBUpdateTask_begin();
	for( int i = 0; i < length; i++)
	{
		int t = i;
		if( tasks_vec.size())
		{
			t = tasks_vec[i];
			if(( t >= m_data->getTasksNum()) || ( t < 0 ))
			{
				appendJobLog("Operation '"+i_type+"' invalid task numer="+af::itos(t)+" by "+i_action.author);
				break;
			}
		}

		if( skip )
			m_tasks[t]->skip( message, i_action.renders, i_action.monitors);
		else
			m_tasks[t]->restart( only_running, message, i_action.renders, i_action.monitors);
	}
	AFCommon::QueueDBUpdateTask_end();
}

void Block::constructDependBlocks()
{
	if( m_job->getBlocksNum() <= 1 )
    {
        // There is only one block, no one other to denend on
        return;
    }

	if( m_data->hasDependMask())
    {
        m_dependBlocks.clear();
		for( int bd = 0; bd < m_job->getBlocksNum(); bd++)
        {
            // skip if it is the same block
			if( bd == m_data->getBlockNum() ) continue;

            // store block if name match mask
			if( m_data->checkDependMask( m_job->getBlock(bd)->getName()))
            m_dependBlocks.push_back( bd);
        }
    }

	if( m_data->hasTasksDependMask())
    {
        m_dependTasksBlocks.clear();
		for( int bd = 0; bd < m_job->getBlocksNum(); bd++)
        {
            // skip if it is the same block
			if( bd == m_data->getBlockNum() ) continue;

            // store block if name match mask
			if( m_data->checkTasksDependMask( m_job->getBlock(bd)->getName()))
            m_dependTasksBlocks.push_back( bd);
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
