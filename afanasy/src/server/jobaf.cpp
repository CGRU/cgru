#include "jobaf.h"

#include "../include/afanasy.h"

#include "../libafanasy/addresseslist.h"
#include "../libafanasy/blockdata.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mclistenaddress.h"
#include "../libafanasy/msgqueue.h"

#include "../libafsql/dbattr.h"
#include "../libafsql/dbconnection.h"
#include "../libafsql/dbjobprogress.h"

#include "action.h"
#include "afcommon.h"
#include "aflistit.h"
#include "block.h"
#include "jobcontainer.h"
#include "monitorcontainer.h"
#include "renderaf.h"
#include "rendercontainer.h"
#include "sysjob.h"
#include "task.h"
#include "useraf.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

JobContainer *JobAf::ms_jobs  = NULL;

JobAf::JobAf( af::Msg * msg):
    afsql::DBJob(),
	AfNodeSrv( this),
    m_fromdatabase( false)
{
    initializeValues();
    read( msg);
    progress = new afsql::DBJobProgress( this);
    construct();
}

JobAf::JobAf( JSON & i_object):
    afsql::DBJob(),
	AfNodeSrv( this),
	m_fromdatabase( false)
{
	initializeValues();
    jsonRead( i_object);
    progress = new afsql::DBJobProgress( this);
    construct();
}

JobAf::JobAf( int Id):
    afsql::DBJob( Id),
	AfNodeSrv( this),
    m_fromdatabase( true)
{
    initializeValues();
}

void JobAf::initializeValues()
{
    m_user             = NULL;
    m_blocks           = NULL;
    m_constructed      = false;
    m_initialized      = false;
    m_deletion         = false;
    m_logsWeight       = 0;
    m_blackListsWeight = 0;
}

bool JobAf::v_dbSelect( PGconn * i_conn, const std::string * i_where)
{
//printf("JobAf::dbSelect:\n");
    if( afsql::DBJob::v_dbSelect( i_conn) == false) return false;
    return construct();
}

bool JobAf::construct()
{
AFINFA("JobAf::construct: \"%s\":", m_name.c_str())

	if( false == isValid())
		return false;

    if( m_constructed )
    {
        AFERROR("JobAf::construct: Already constructed.")
        return true;
    }

    if( m_blocksnum < 1 )
    {
        AFERRAR("JobAf::construct: invalid number of blocks = %d", m_blocksnum)
        return false;
    }

    m_blocks = new Block*[m_blocksnum];
    if( m_blocks == NULL )
    {
        AFERROR("JobAf::construct: Can't allocate memory for blocks.")
        return false;
    }
    for( int b = 0; b < m_blocksnum; b++) m_blocks[b] = NULL;
    for( int b = 0; b < m_blocksnum; b++)
    {
        if( m_blocksdata[b]->isValid() == false) return false;
        m_blocks[b] = v_newBlock(b);
        if( m_blocks[b] == NULL )
        {
            AFERRAR("JobAf::construct: Can't create block %d of %d.", b, m_blocksnum)
            return false;
        }
    }

    m_constructed = true;
    return true;
}

JobAf::~JobAf()
{
    if( m_blocks )
    {
        for( int b = 0; b < m_blocksnum; b++) if( m_blocks[b]) delete m_blocks[b];
        delete [] m_blocks;
    }
}

Block * JobAf::v_newBlock( int numBlock)
{
	return new Block( this, m_blocksdata[numBlock], progress);
}

void JobAf::setUser( UserAf * i_user)
{
    m_user = i_user;
    for( int b = 0; b < m_blocksnum; b++)
    {
        m_blocks[b]->setUser( i_user);
    }
    m_user_name = i_user->getName();
}

bool JobAf::initialize()
{
   if( isConstructed() == false)
   {
      AFERROR("JobAf::initialize: Job is not constructed.")
      return 0;
   }

//printf("JobAf::initialize: BEGIN\n");
   progress->setJobId( m_id);
   for( int b = 0; b < m_blocksnum; b++)
   {
      m_blocksdata[b]->setJobId( m_id);
   }

//
// Create tasks output directory ( if needed )
   m_tasks_output_dir = m_name;
   af::pathFilterFileName( m_tasks_output_dir);
   m_tasks_output_dir = af::Environment::getTasksStdOutDir() + AFGENERAL::PATH_SEPARATOR + m_tasks_output_dir;
   if( af::pathMakeDir( m_tasks_output_dir) == false)
   {
      appendLog( std::string("Unable to create tasks output directory:\n") + m_tasks_output_dir);
      return false;
   }

//
// Executing pre commands ( if not from database )

   if( m_fromdatabase == false )
   {
      if( false == m_command_pre.empty())
      {
         AFCommon::executeCmd( m_command_pre);
         appendLog( std::string("Job pre command executed:\n") + m_command_pre);
      }
      for( int b = 0; b < m_blocksnum; b++)
      {
         if( m_blocksdata[b]->hasCmdPre() )
         {
            AFCommon::executeCmd( m_blocksdata[b]->getCmdPre());
            appendLog( std::string("Block[") + m_blocksdata[b]->getName() + "] pre command executed:\n" + m_blocksdata[b]->getCmdPre());
         }
      }
      appendLog("Initialized.");
   }
   else
   {
      appendLog("Initialized from database.");
   }

//
// Checking states

   for( int b = 0; b < m_blocksnum; b++)
   {
//      if( b == 0 ) blocksdata[b]->state = blocksdata[b]->state | AFJOB::STATE_READY_MASK;
//      blocksdata[b]->state = blocksdata[b]->state & (~AFJOB::STATE_WAITDEP_MASK);
      int numtasks = m_blocksdata[b]->getTasksNum();
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

   if(( m_state & AFJOB::STATE_DONE_MASK) == false ) m_state = m_state | AFJOB::STATE_WAITDEP_MASK;

   v_refresh( time(NULL), NULL, NULL);

   m_initialized = true;

//printf("JobAf::initialize: END\n");
   return true;
}

int JobAf::getUid() const { return m_user->getId(); }

void JobAf::v_setZombie( RenderContainer * renders, MonitorContainer * monitoring)
{
   if( m_deletion == false )
   {
      m_state = AFJOB::STATE_OFFLINE_MASK;
      lock();
      m_deletion = true;
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

   if( false == m_command_post.empty())
   {
      SysJob::AddPostCommand( m_command_post, m_blocksnum > 0 ? m_blocksdata[0]->getWDir(): "", m_user_name, m_name);
      appendLog( std::string("Executing job post command:\n") + m_command_post);
   }
   for( int b = 0; b < m_blocksnum; b++)
   {
      if( m_blocksdata[b]->hasCmdPost())
      {
         SysJob::AddPostCommand( m_blocksdata[b]->getCmdPost(), m_blocksdata[b]->getWDir(), m_user_name, m_name);
         appendLog( std::string("Executing block[") + m_blocksdata[b]->getName() + "] post command:\n" + m_blocksdata[b]->getCmdPost());
      }
   }
   AfNodeSrv::v_setZombie();

   // Queue job cleanup:
   AFCommon::QueueJobCleanUp( this);

   if( isInitialized()) AFCommon::QueueDBDelItem( this);
   if( monitoring ) monitoring->addJobEvent( af::Msg::TMonitorJobsDel, getId(), getUid());
   AFCommon::QueueLog("Deleting a job: " + v_generateInfoString());
   unLock();
}

void JobAf::v_action( Action & i_action)
{
	// If action has blocks ids array - action to for blocks
	if( i_action.data->HasMember("block_ids"))
	{
		const JSON & blocks = (*i_action.data)["block_ids"];
		if( blocks.IsArray())
		{
			std::vector<int32_t> block_ids;
			af::jr_int32vec("block_ids", block_ids, *i_action.data);
			if( block_ids.size())
			{
				bool job_progress_changed = false;
				// If blocks ids array has only one "-1" value - action to for all blocks
				if(( block_ids.size() == 1 ) && ( block_ids[0] == -1 ))
				{
					for( int b = 0; b < m_blocksnum; b++)
						if( m_blocks[b]->action( i_action))
							job_progress_changed = true;
				}
				else
				{
					for( int b = 0; b < block_ids.size(); b++)
					{
						if(( block_ids[b] >= getBlocksNum()) || ( block_ids[b] < 0 ))
						{
							appendLog("Invalid block number = " + af::itos(block_ids[b]) + " " + i_action.author);
							continue;
						}
						if( m_blocks[block_ids[b]]->action( i_action))
							job_progress_changed = true;
					}
				}

				if( job_progress_changed )
					i_action.monitors->addJobEvent( af::Msg::TMonitorJobsChanged, getId(), getUid());

				return;
			}
			else
			{
				appendLog("\"block_ids\" array does not contain any integers " + i_action.author);
				return;
			}
		}
		else
		{
			appendLog("\"block_ids\" should be an array of integers " + i_action.author);
			return;
		}
		return;
	}

	const JSON & operation = (*i_action.data)["operation"];
	if( operation.IsObject())
	{
		std::string type;
		af::jr_string("type", type, operation);
		if( type == "delete")
		{
			appendLog("Deleted by " + i_action.author);
			m_user->appendLog( "Job \"" + m_name + "\" deleted by " + i_action.author);
			v_setZombie( i_action.renders, i_action.monitors);
			i_action.monitors->addJobEvent( af::Msg::TMonitorJobsDel, getId(), getUid());
			return;
		}
		else if( type == "start")
		{
			m_state = m_state & (~AFJOB::STATE_OFFLINE_MASK);
		}
		else if( type == "pause")
		{
			m_state = m_state | AFJOB::STATE_OFFLINE_MASK;
		}
		else if( type == "stop")
		{
		   restartAllTasks( true, "Job stopped by " + i_action.author, i_action.renders, i_action.monitors);
		   m_state = m_state | AFJOB::STATE_OFFLINE_MASK;
		}
		else if( type == "restart_running")
		{
			restartAllTasks( true, "Job restarted running by " + i_action.author,  i_action.renders, i_action.monitors);
		}
		else if( type == "reset_error_hosts")
		{
			for( int b = 0; b < m_blocksnum; b++)
				m_blocks[b]->action( i_action);
		}
		else if( type == "restart")
		{
			//printf("Msg::TJobRestart:\n");
			restartAllTasks( false, "Job restarted by " + i_action.author,  i_action.renders, i_action.monitors);
			//printf("Msg::TJobRestart: tasks restarted.\n");
			checkDepends();
			m_time_started = 0;
		}
		else if( type == "restart_errors")
		{
			restartErrors( "Job errors restarted by " + i_action.author,  i_action.renders, i_action.monitors);
		}
		else if( type == "restart_pause")
		{
			restartAllTasks( false, "Job restarted ( and paused ) by " + i_action.author,  i_action.renders, i_action.monitors);
			checkDepends();
			m_state = m_state | AFJOB::STATE_OFFLINE_MASK;
			m_time_started = 0;
		}
		else
		{
			appendLog("Unknown operation \"" + type + "\" by " + i_action.author);
			return;
		}
		appendLog("Operation \"" + type + "\" by " + i_action.author);
		i_action.monitors->addJobEvent( af::Msg::TMonitorJobsChanged, getId(), getUid());
		AFCommon::QueueDBUpdateItem( this);
		return;
	}

	// Store user name before parameters read, to check whether it changed
	const std::string user_name = m_user_name;

	const JSON & params = (*i_action.data)["params"];
	if( params.IsObject())
		jsonRead( params, &i_action.log);

	if( m_user_name != user_name )
	{
		// User name was changed
        UserAf * user = UserContainer::getUser( m_user_name);
        if( user == NULL )
		{
			return;
		}

        i_action.monitors->addEvent(    af::Msg::TMonitorUsersChanged, m_user->getId());
        i_action.monitors->addJobEvent( af::Msg::TMonitorJobsDel, getId(), m_user->getId());

        m_user->removeJob( this);
        user->addJob( this);

        i_action.monitors->addEvent(    af::Msg::TMonitorUsersChanged, m_user->getId());
        i_action.monitors->addJobEvent( af::Msg::TMonitorJobsAdd, getId(), m_user->getId());

        AFCommon::QueueDBUpdateItem( this);

		return;
	}

	if( i_action.log.size() )
	{
		AFCommon::QueueDBUpdateItem( this);
		i_action.monitors->addJobEvent( af::Msg::TMonitorJobsChanged, getId(), getUid());
	}
}

void JobAf::v_priorityChanged( MonitorContainer * i_monitoring)
{
	if( i_monitoring ) i_monitoring->addUser( m_user);
	ms_jobs->sortPriority( this);
}

void JobAf::setUserListOrder( int index, bool updateDtabase)
{
   int old_index = m_user_list_order;
   m_user_list_order = index;
   if(( index != old_index ) && updateDtabase ) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_userlistorder);
}

void JobAf::checkDepends()
{
   m_state = m_state & (~AFJOB::STATE_WAITDEP_MASK);

//printf("JobAf::checkDepends: name1=%s\n", name.toUtf8().data());
   bool depend_local = false;
   bool depend_global = false;

   // check global depends:
   if( hasDependMaskGlobal())
   {
	  JobContainerIt jobsIt( ms_jobs);
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
      AfListIt jobsListIt( m_user->getJobsList());
      for( AfNodeSrv *job = jobsListIt.node(); job != NULL; jobsListIt.next(), job = jobsListIt.node())
      {
         if( job == this ) continue;
//printf("JobAf::checkDepends: name2=%s\n", job->getName().toUtf8().data());
         if(( ((JobAf*)job)->isDone() == false ) && ( checkDependMask( ((JobAf*)job)->getName()) ))
         {
            depend_local = true;
//printf("Set.\n");
            break;
         }
      }
   }

   if( depend_local || depend_global ) m_state = m_state | AFJOB::STATE_WAITDEP_MASK;
}

void JobAf::addRenderCounts( RenderAf * render)
{
   std::list<RenderAf*>::iterator rit = renders_ptrs.begin();
   std::list<int>::iterator cit = renders_counts.begin();
   for( ; rit != renders_ptrs.end(); rit++, cit++)
      if( render == *rit )
      {
         (*cit)++;
         return;
      }
   renders_ptrs.push_back( render);
   renders_counts.push_back( 1);
}

int JobAf::getRenderCounts( RenderAf * render) const
{
   std::list<RenderAf*>::const_iterator rit = renders_ptrs.begin();
   std::list<int>::const_iterator cit = renders_counts.begin();
   for( ; rit != renders_ptrs.end(); rit++, cit++)
      if( render == *rit ) return *cit;
   return 0;
}

void JobAf::remRenderCounts( RenderAf * render)
{
   std::list<RenderAf*>::iterator rit = renders_ptrs.begin();
   std::list<int>::iterator cit = renders_counts.begin();
   for( ; rit != renders_ptrs.end(); rit++, cit++)
      if( render == *rit )
      {
         if( *cit > 1 )
            (*cit)--;
         else
         {
            renders_ptrs.erase( rit);
            renders_counts.erase( cit);
         }
         return;
      }
}

af::TaskExec * JobAf::genTask( RenderAf *render, int block, int task, std::list<int> * blocksIds, MonitorContainer * monitoring)
{
   // Job can set offline itself on some error in this recursive function
    if( m_state & AFJOB::STATE_OFFLINE_MASK )
        return NULL;

	if( m_blocks[block]->m_tasks[task]->m_solved )
        return NULL;
	m_blocks[block]->m_tasks[task]->m_solved = true;

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
            appendLog( std::string("Block[") + m_blocksdata[block]->getName() + "] appears second time while job generating a task.\nJob has a recursive blocks tasks dependence.");
            m_state = m_state | AFJOB::STATE_OFFLINE_MASK;
            if( monitoring ) monitoring->addJobEvent( af::Msg::TMonitorJobsChanged, getId(), getUid());
            return NULL;
         }
         bIt++;
      }
      blocksIds->push_back( block);
   }
   //

   if( false == ( m_blocksdata[block]->getState() & AFJOB::STATE_READY_MASK ) ) return NULL;
   if( task >= m_blocksdata[block]->getTasksNum() )
   {
      AFERRAR("JobAf::genTask: block[%d] '%s' : %d >= number of tasks = %d.",
         block, m_blocksdata[block]->getName().c_str(), task, m_blocksdata[block]->getTasksNum())
      return NULL;
   }
   if( false == ( progress->tp[block][task]->state & AFJOB::STATE_READY_MASK) ) return NULL;

   if( false == m_blocks[block]->canRunOn( render)) return NULL;

   if( m_blocks[block]->m_tasks[task]->avoidHostsCheck( render->getName())) return NULL;

   //
   // Check block tasks dependence: Get tasks depend mask, if any exists:
   if( m_blocksdata[block]->hasTasksDependMask() )
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
      for( int b = 0; b < m_blocksnum; b++)
      {
         if( b == block ) continue;

 //         if( blocksdata[block]->checkTasksDependMask( blocksdata[b]->getName()) == false ) continue;
         if( m_blocks[block]->tasksDependsOn( b) == false )
             continue;

         long long firstdependframe, lastdependframe;
         int firstdependtask, lastdependtask;
         m_blocksdata[block]->genNumbers( firstdependframe, lastdependframe, task);
         if( m_blocksdata[block]->isNumeric() && m_blocksdata[b]->isNotNumeric())
         {
            firstdependframe -= m_blocksdata[block]->getFrameFirst();
            lastdependframe  -= m_blocksdata[block]->getFrameFirst();
         }
         else if( m_blocksdata[block]->isNotNumeric() && m_blocksdata[b]->isNumeric())
         {
            firstdependframe += m_blocksdata[b]->getFrameFirst();
            lastdependframe  += m_blocksdata[b]->getFrameFirst();
         }
         if( m_blocksdata[b]->getFramePerTask() < 0 ) lastdependframe++; // For several frames in task

			bool inValidRange;
			firstdependtask = m_blocksdata[b]->calcTaskNumber( firstdependframe, inValidRange);
			lastdependtask  = m_blocksdata[b]->calcTaskNumber(  lastdependframe, inValidRange);
			if( inValidRange )
				if( m_blocksdata[b]->getFramePerTask() < 0 )
					lastdependtask--;

//printf("Dep['%s'[%d]/(%lld) <- '%s'/(%lld)]: DepFrames = %lld - %lld: DepTasks = %d - %d\n", blocksdata[block]->getName().c_str(), task, blocksdata[block]->getFramePerTask(), blocksdata[b]->getName().c_str(), blocksdata[b]->getFramePerTask(), firstdependframe, lastdependframe, firstdependtask, lastdependtask);

         for( int t = firstdependtask; t <= lastdependtask; t++)
         {
//printf("Dep['%s':%d-'%s']: checking '%s':%d - %s\n", blocksdata[block]->getName().toUtf8().data(), task, blocksdata[b]->getName().toUtf8().data(), blocksdata[b]->getName().toUtf8().data(), t, (progress->tp[b][t]->state & AFJOB::STATE_DONE_MASK) ? "DONE" : "NOT Done");
            // Task is done, so depend is satisfied:
            if( progress->tp[b][t]->state & AFJOB::STATE_DONE_MASK )
					continue;

            // Check subframe depend, is depend task is running:
            if( m_blocksdata[b]->isDependSubTask() && ( progress->tp[b][t]->state & AFJOB::STATE_RUNNING_MASK ))
            {
//               long long f_start, f_end, f_start_dep, f_end_dep;
//               m_blocksdata[block]->genNumbers( f_start, f_end, task);
               long long f_start_dep, f_end_dep;
               m_blocksdata[b]->genNumbers( f_start_dep, f_end_dep, t);
               long long frame_run = f_start_dep + progress->tp[b][t]->frame;
//printf("Dep['%s': #%d '%s']: f_s=%lld f_d=%lld\n", m_blocksdata[block]->getName().c_str(), task, m_blocksdata[b]->getName().c_str(), firstdependframe, frame_run);
               if( frame_run > lastdependframe )
					continue;
            }

            // Run recurtsion:
            af::TaskExec * task_ptr = genTask( render, b, t, (t == firstdependtask ? blocksIds : NULL), monitoring);
            if( m_state & AFJOB::STATE_OFFLINE_MASK )
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

	af::TaskExec * task_exec = m_blocksdata[block]->genTask( task);

	task_exec->m_custom_data_job = m_custom_data;
	task_exec->m_custom_data_render = render->getCustomData();
	task_exec->m_custom_data_user = m_user->getCustomData();

	return task_exec;
}

bool JobAf::v_canRun()
{
	if( isLocked() )
    {
        return false;
    }

	// Check some validness:
    if( m_blocksnum < 1)
    {
        AFERROR("JobAf::solve: job has no blocks.")
        return false;
    }

	// check job state:
    if( m_state & AFJOB::STATE_OFFLINE_MASK )
    {
        return false;
    }

	if( false == ( m_state & AFJOB::STATE_READY_MASK  ))
    {
		return false;
    }

	// Zero priority turns job off:
    if( m_priority == 0 )
    {
        return false;
    }

	// check maximum running tasks:
    if(( m_max_running_tasks >= 0 ) && ( getRunningTasksNumber() >= m_max_running_tasks ))
    {
        return false;
    }

	// check maximum running tasks per host:
    if(  m_max_running_tasks_per_host == 0 )
    {
        return false;
    }

    return true;
}

bool JobAf::v_canRunOn( RenderAf * i_render)
{
    if( false == v_canRun())
    {
        // Unable to run at all
        return false;
    }

    // check maximum running tasks per host:
    if(( m_max_running_tasks_per_host  > 0 ) && ( getRenderCounts(i_render) >= m_max_running_tasks_per_host ))
    {
        return false;
    }

	// check at least one block can run on render
	bool blockCanRunOn = false;
	for( int b = 0; b < m_blocksnum; b++)
	{
//		if( i_render->hasCapacity( m_blocksdata[b]->getCapMinResult()))
		if( m_blocks[b]->canRunOn( i_render))
		{
			blockCanRunOn = true;
			break;
		}
	}
	if( false == blockCanRunOn )
		return false;

	// check hosts mask:
    if( false == checkHostsMask( i_render->getName()))
    {
        return false;
    }

	// check exclude hosts mask:
    if( false == checkHostsMaskExclude( i_render->getName()))
    {
        return false;
    }

	// check needed os:
    if( false == checkNeedOS( i_render->getHost().m_os))
    {
        return false;
    }

	// check needed properties:
    if( false == checkNeedProperties( i_render->getHost().m_properties))
    {
        return false;
    }

    return true;
}

bool JobAf::v_solve( RenderAf *render, MonitorContainer * monitoring)
{
//printf("Job::solve: %s:\n", m_name.c_str());
	for( int b = 0; b < m_blocksnum; b++)
    {
        int numtasks = m_blocksdata[b]->getTasksNum();
        for( int t = 0; t < numtasks; t++)
        {
			// Needed for recursion function, to not to try to solve the same task again
			m_blocks[b]->m_tasks[t]->m_solved = false;

			if( m_blocksdata[b]->isNonSequential())
			{
				// Needed to store tasks that was tried
				progress->tp[b][t]->setNotSolved();
			}
        }
    }

	for( int b = 0; b < m_blocksnum; b++)
	{
		if( false == ( m_blocksdata[b]->getState() & AFJOB::STATE_READY_MASK )) continue;

		int numtasks = m_blocksdata[b]->getTasksNum();
		int t = -1;
		for(;;)
		{
			if( m_blocksdata[b]->isSequential())
			{
				t++;
				if( t >= numtasks )
				{
					break;
				}

				if( false == ( progress->tp[b][t]->state & AFJOB::STATE_READY_MASK ))
				{
					continue;
				}
			}
			else
			{
				t = af::getReadyTaskNumber( numtasks, progress->tp[b], m_blocksdata[b]->getFlags());
				if( t == -1 )
					break;
			}

			//static int cycle = 0;printf("cycle = %d\n", cycle++);
			std::list<int> blocksIds;
			af::TaskExec *task_exec = genTask( render, b, t, &blocksIds, monitoring);

			// Job may became paused, if recursion during task generation detected:
			if( m_state & AFJOB::STATE_OFFLINE_MASK )
			{
				if( task_exec ) delete task_exec;
				return false;
			}

			// Check if render is online
			// It can be solved with offline render to check whether to WOL wake it
			if( task_exec && render->isOffline())
			{
				delete task_exec;
				return true;
			}

			// No task was generated:
			if( task_exec == NULL ) continue;

			// Job successfully solved (produced a task)
			task_exec->setJobName( m_name);
			task_exec->setUserName( m_user_name);
			listeners.process( *task_exec);
			m_blocks[task_exec->getBlockNum()]->v_startTask( task_exec, render, monitoring);

			// If job was not started it became started
			if( m_time_started == 0 )
			{
				m_time_started = time(NULL);
				appendLog("Started.");
				AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_time_started);
			}

			return true;
		}
	}
	return false;
}

void JobAf::v_updateTaskState( const af::MCTaskUp& taskup, RenderContainer * renders, MonitorContainer * monitoring)
{
   if( false == checkBlockTaskNumbers( taskup.getNumBlock(), taskup.getNumTask(), "updateTaskState")) return;
   bool errorHost = false;
   m_blocks[taskup.getNumBlock()]->m_tasks[taskup.getNumTask()]->v_updateState( taskup, renders, monitoring, errorHost);
   if( errorHost) m_blocks[taskup.getNumBlock()]->v_errorHostsAppend( taskup.getNumTask(), taskup.getClientId(), renders);
}

void JobAf::v_refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
//printf("JobAf::refresh: \"%s\"\n", getName().toUtf8().data());
   RenderContainer * renders = (RenderContainer*)pointer;

   if( m_deletion )
   {
//printf("JobAf::refresh: deletion: runningtaskscounter = %d\n", runningtaskscounter);
      for( int b = 0; b < m_blocksnum; b++)
         m_blocks[b]->v_refresh( currentTime, renders, monitoring);
      if( getRunningTasksNumber() == 0 ) v_setZombie( NULL, monitoring);
//printf("JobAf::refresh: deletion: runningtaskscounter = %d\n", runningtaskscounter);
   }
   if( isLocked() ) return;

   // for database and monitoring
   uint32_t old_state = m_state;
   uint32_t jobchanged = 0;

   // check job dependences
   checkDepends();

   //check wait time
   {
      bool wasWaiting = m_state & AFJOB::STATE_WAITTIME_MASK;
      if( m_time_wait > currentTime ) m_state = m_state |   AFJOB::STATE_WAITTIME_MASK;
      else                          m_state = m_state & (~AFJOB::STATE_WAITTIME_MASK);
      bool nowWaining = m_state & AFJOB::STATE_WAITTIME_MASK;
      if( wasWaiting != nowWaining) jobchanged = af::Msg::TMonitorJobsChanged;
   }

   //
   // Update blocks (blocks will uptate its tasks):
   for( int b = 0; b < m_blocksnum; b++)
      if( m_blocks[b]->v_refresh( currentTime, renders, monitoring))
         jobchanged = af::Msg::TMonitorJobsChanged;

   //
   // job state calculation
   m_state = m_state |   AFJOB::STATE_DONE_MASK;
   m_state = m_state & (~AFJOB::STATE_RUNNING_MASK);
   m_state = m_state & (~AFJOB::STATE_ERROR_MASK);
   m_state = m_state & (~AFJOB::STATE_READY_MASK);
   m_state = m_state & (~AFJOB::STATE_SKIPPED_MASK);

   for( int b = 0; b < m_blocksnum; b++)
   {
      uint32_t state_block = m_blocksdata[b]->getState();
      m_state  = m_state | (state_block &   AFJOB::STATE_RUNNING_MASK   );
      m_state  = m_state | (state_block &   AFJOB::STATE_ERROR_MASK     );
      m_state  = m_state | (state_block &   AFJOB::STATE_READY_MASK     );
      m_state  = m_state | (state_block &   AFJOB::STATE_SKIPPED_MASK   );
      m_state  = m_state & (state_block | (~AFJOB::STATE_DONE_MASK)     );
   }

   if( m_state & AFJOB::STATE_WAITDEP_MASK  ) m_state = m_state & (~AFJOB::STATE_READY_MASK);
   if( m_state & AFJOB::STATE_WAITTIME_MASK ) m_state = m_state & (~AFJOB::STATE_READY_MASK);

   if( m_state & AFJOB::STATE_DONE_MASK )
   {
      /// if job was not done, but now is done, we set job time_done
      if(( old_state & AFJOB::STATE_DONE_MASK) == false )
      {
         m_time_done = currentTime;
         if( m_time_started == 0 )
         {
            m_time_started = m_time_done;
            appendLog("Started.");
         }
         appendLog("Done.");
         jobchanged = af::Msg::TMonitorJobsChanged;
         AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_time_done);
      }
   }
   else
      m_time_done = 0;

   // Reset started time if job was started, but now no tasks are running or done
   if(( m_time_started != 0 ) &&
      ( false == (m_state & AFJOB::STATE_RUNNING_MASK)     ) &&
      ( false == (m_state & AFJOB::STATE_DONE_MASK)        )  )
   {
      // Search if the job has at least one done task
      bool has_done_tasks = false;
      for( int b = 0; b < m_blocksnum; b++ )
      {
         if( m_blocksdata[b]->getProgressTasksDone() > 0 )
         {
            has_done_tasks = true;
            break;
         }
      }
      // If the job has done task(s) we not reset started time in any case
      if( false == has_done_tasks ) m_time_started = currentTime;
   }


	if( m_state != old_state )
	{
		jobchanged = af::Msg::TMonitorJobsChanged;

		// If it is no job monitoring, job just came to server and it is first it refresh,
		// so no change event and database storing needed
		if( monitoring ) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);

		// Proccess events:
		if( m_id != AFJOB::SYSJOB_ID ) // skip system job
		if( m_custom_data.size() || m_user->getCustomData().size())
		{
			std::vector<std::string> events;

			// Collect events names:
			if(( m_state & AFJOB::STATE_ERROR_MASK ) && ( false == ( old_state & AFJOB::STATE_ERROR_MASK )))
				events.push_back("JOB_ERROR");

			if(( m_state & AFJOB::STATE_DONE_MASK ) && ( false == ( old_state & AFJOB::STATE_DONE_MASK )))
				events.push_back("JOB_DONE");

			// Processing command for system job is some events happened:
			if( events.size())
			{
				std::string cmd = "{\n";
				if( m_user->getCustomData().size()) cmd += "\"user\":" + m_user->getCustomData();
				if( m_custom_data.size()) cmd += ",\n\"job\":" + m_custom_data;
				cmd += ",\n\"events\":[";
				for( int i = 0; i < events.size(); i++ )
				{
					if( i ) cmd += ',';
					cmd = cmd + '"' + events[i] + '"';
				}
				cmd += "]\n}";

				SysJob::AddEventCommand( cmd,
					"", // working directory - no matter
					m_user_name, m_name, events[0]);
			}
		}
	}

   // Check age and delete if life finished:
   if( m_id != AFJOB::SYSJOB_ID ) // skip system job
   {
      int result_lifetime = m_time_life;
      if( result_lifetime < 0 ) result_lifetime = m_user->getJobsLifeTime(); // get default value from user
      if((result_lifetime > 0) && ((currentTime - m_time_creation) > result_lifetime))
      {
         appendLog( std::string("Life %1 finished.") + af::time2strHMS( result_lifetime, true));
         m_user->appendLog( std::string("Job \"") + m_name + "\" life " + af::time2strHMS( result_lifetime, true) + " finished.");
         v_setZombie( renders, monitoring);
         jobchanged = af::Msg::TMonitorJobsDel, getId(), getUid();
      }
   }

   if(( monitoring ) &&  ( jobchanged )) monitoring->addJobEvent( jobchanged, getId(), getUid());

   // Update solving parameters:
   v_calcNeed();
}

void JobAf::v_calcNeed()
{
    // Need calculation based on running tasks number
    calcNeedResouces( getRunningTasksNumber());
}

void JobAf::skipTasks( const af::MCTasksPos &taskspos, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("JobAf::skipTasks:\n");
   tasks_Skip_Restart( taskspos, false,renders, monitoring);
//printf("JobAf::skipTasks: tasks skipped.\n");
}

void JobAf::v_restartTasks( const af::MCTasksPos &taskspos, RenderContainer * renders, MonitorContainer * monitoring)
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
      if( b < 0)
      {
         AFERROR("JobAf::tasks_Skip_Restart: b < 0")
         continue;
      }
      if( b >= m_blocksnum)
      {
         AFERRAR("JobAf::tasks_Skip_Restart: b >= blocksnum ( %d >= %d )", b, m_blocksnum)
         continue;
      }
      int start, end;
      if( taskspos.hasTasks())
      {
         start = taskspos.getNumTask( p);
         if( start >= m_blocksdata[b]->getTasksNum())
         {
            AFERRAR("JobAf::tasks_Skip_Restart: taskspos.getNumTask() >= numTasks, ( %d >= %d )", start, m_blocksdata[b]->getTasksNum())
            continue;
         }
         if( start < 0)
         {
            AFERROR("JobAf::tasks_Skip_Restart: taskspos.getNumTask() < 0")
            continue;
         }
         end = start+1;
      }
      else
      {
         start = 0;
         end = m_blocksdata[b]->getTasksNum();
      }

      std::string message;
      if( restart) message = "Restart request by ";
      else         message = "Skip request by ";
      message += taskspos.getUserName() + '@' + taskspos.getHostName() + ' ' + taskspos.getMessage();

      for( int t = start; t < end; t++)
      {
		 if( restart) m_blocks[b]->m_tasks[t]->restart( false, message, renders, monitoring);
		 else         m_blocks[b]->m_tasks[t]->skip( message, renders, monitoring);
      }
   }
   AFCommon::QueueDBUpdateTask_end();
}

void JobAf::restartAllTasks( bool onlyRunning, const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
{
   AFCommon::QueueDBUpdateTask_begin();
   for( int b = 0; b < m_blocksnum; b++)
   {
      int numtasks = m_blocksdata[b]->getTasksNum();
      for( int t = 0; t < numtasks; t++)
		 m_blocks[b]->m_tasks[t]->restart( onlyRunning, message, renders, monitoring);
   }
   AFCommon::QueueDBUpdateTask_end();
   v_refresh( time(NULL), renders, monitoring);
}

void JobAf::restartErrors( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
{
   for( int b = 0; b < m_blocksnum; b++)
   {
      int numtasks = m_blocksdata[b]->getTasksNum();
      for( int t = 0; t < numtasks; t++)
		 m_blocks[b]->m_tasks[t]->restartError( message, renders, monitoring);
   }
}

void JobAf::writeProgress( af::Msg &msg)
{
   msg.set( af::Msg::TJobProgress, progress);
}

af::Msg * JobAf::writeProgress( bool json)
{
	af::Msg * msg = new af::Msg(); 
	if( json )
	{
		std::ostringstream stream;
		progress->jsonWrite( stream);
		std::string string = stream.str();
		msg->setData( string.size(), string.c_str(), af::Msg::TJSON);
	}
	else
	{
		msg->set( af::Msg::TJobProgress, progress);
	}

	return msg;
}

af::Msg * JobAf::writeBlocks( std::vector<int32_t> i_block_ids, std::vector<std::string> i_modes) const
{
//printf("JobAf::writeBlocks: bs=%d, ms=%d\n", i_block_ids.size(), i_modes.size());
//printf("bids:");for(int i=0;i<i_block_ids.size();i++)printf(" %d",i_block_ids[i])    ;printf("\n");
//printf("mods:");for(int i=0;i<i_modes.size()    ;i++)printf(" %s",i_modes[i].c_str());printf("\n");
	if( i_block_ids.size() != i_modes.size())
	{
		AFERRAR("JobAf::writeBlocks: i_block_ids.size() != i_modes.size(): %d != %d",
				int(i_block_ids.size()), int(i_modes.size()));
		return NULL;
	}

	std::ostringstream str;
	str << "{\"blocks\":[\n";
	for( int b = 0; b < i_block_ids.size(); b++)
	{
		if( b > 0 ) str << ",";
		if( i_block_ids[b] >= m_blocksnum )
		{
			AFERRAR("JobAf::writeBlocks: i_block_ids[b] >= m_blocksnum: %d >= %d",
				i_block_ids[b], m_blocksnum);
			return NULL;
		}
		m_blocksdata[i_block_ids[b]]->jsonWrite( str, i_modes[b]);
	}
	str << "\n]}";

	return af::jsonMsg( str);
}

af::Msg * JobAf::writeTask( int i_b, int i_t, const std::string & i_mode) const
{
	std::ostringstream str;
	str << "{";

	if( i_mode == "log" )
		return af::jsonMsg( "log", getName()+"["+af::itos(i_b)+","+af::itos(i_t)+"]", getTaskLog( i_b, i_t));
	else if( i_mode == "info" )
	{
		af::TaskExec * task = generateTask( i_b, i_t);
		if( task )
			task->jsonWrite( str, af::Msg::TTask);
	}
	else if( i_mode == "output")
	{
	}

	str << "}";
	return af::jsonMsg( str);
}

const std::list<std::string> & JobAf::getTaskLog( int block, int task) const
{
   static const std::list<std::string> emptylog;
   if( false == checkBlockTaskNumbers( block, task, "getTaskLog")) return emptylog;
   return m_blocks[block]->m_tasks[task]->getLog();
}

af::TaskExec * JobAf::generateTask( int block, int task) const
{
   if( false == checkBlockTaskNumbers( block, task, "generateTask")) return NULL;
   return m_blocks[block]->m_data->genTask( task);
}

const std::string JobAf::generateTaskName( int i_b, int i_t) const
{
	if( false == checkBlockTaskNumbers( i_b, i_t, "generateTaskName")) return "Invalid ids.";
	else return m_blocks[i_b]->m_data->genTaskName( i_t);
}

const std::string JobAf::v_getErrorHostsListString() const
{
   std::string str("Job \"");
   str += m_name + "\" error hosts:\n";
	std::list<std::string> list;
	writeErrorHosts( list);
	str += af::strJoin( list, "\n");
   return str;
}

void JobAf::writeErrorHosts( std::list<std::string> & o_list) const
{
	for( int block = 0; block < m_blocksnum; block++)
		m_blocks[block]->v_getErrorHostsList( o_list);
}

af::Msg * JobAf::writeErrorHosts() const
{
	std::list<std::string> list;
	writeErrorHosts( list);
	return af::jsonMsg("error_hosts", m_name, list);
}

af::Msg * JobAf::writeErrorHosts( int b, int t) const
{
	if( false == checkBlockTaskNumbers( b, t, "getErrorHostsList"))
	{
		return af::jsonMsgError( std::string("Job '") + m_name + "' invalid task number: " + af::itos(b) + ", " + af::itos(t) + ".");
	}

	std::list<std::string> list;
	m_blocks[b]->m_tasks[t]->getErrorHostsList( list);
	if( list.empty())
		list.push_back("The task has no error hosts.");

	return af::jsonMsg("error_hosts", m_name, list);
}

const std::string JobAf::v_getErrorHostsListString( int b, int t) const
{
   if( false == checkBlockTaskNumbers( b, t, "getErrorHostsList"))
   {
      return std::string("Invalid task[") + af::itos(b) + "][" + af::itos(t) + "].";
   }
   std::string str = m_blocks[b]->m_tasks[t]->getErrorHostsListString();
   if( str.empty()) str = "The task has no error hosts.";
   return str;
}

bool JobAf::checkBlockTaskNumbers( int BlockNum, int TaskNum, const char * str) const
{
   if( BlockNum >= m_blocksnum)
   {
      if( str ) AFERRAR("JobAf::checkBlockTaskNumbers: %s: numblock >= blocksnum ( %d >= %d )", str, BlockNum, m_blocksnum)
      else      AFERRAR("JobAf::checkBlockTaskNumbers: numblock >= blocksnum ( %d >= %d )", BlockNum, m_blocksnum)
      return false;
   }
   if( TaskNum >= m_blocksdata[BlockNum]->getTasksNum())
   {
      if( str ) AFERRAR("JobAf::checkBlockTaskNumbers: %s: numtask >= numTasks ( %d >= %d )", str, TaskNum, m_blocksdata[BlockNum]->getTasksNum())
      else      AFERRAR("JobAf::checkBlockTaskNumbers: numtask >= numTasks ( %d >= %d )", TaskNum, m_blocksdata[BlockNum]->getTasksNum())
      return false;
   }
   return true;
}

af::Msg * JobAf::v_getTaskStdOut( int i_b, int i_t, int i_n, RenderContainer * i_renders,
	std::string & o_filename, std::string & o_error) const
{
//printf("JobAf::getTaskStdOut:\n");
	if( false == checkBlockTaskNumbers( i_b, i_t, "getTaskStdOut"))
	{
		o_error = "Invalid blockb and task numbers";
		return NULL;
	}
	return m_blocks[i_b]->m_tasks[i_t]->getOutput( i_n, i_renders, o_filename, o_error);
}

void JobAf::listenOutput( af::MCListenAddress & mclisten, RenderContainer * renders)
{
//printf("JobAf::listenOutput: (%s) ", onoff?"ON":"OFF");address->stdOut();printf(", jobid=%d:\n", id);
   if( listeners.process( mclisten) == false ) return;

   if( mclisten.justTask())
   {
      checkBlockTaskNumbers( mclisten.getNumBlock(), mclisten.getNumTask(), "listenOutput");
	  m_blocks[mclisten.getNumBlock()]->m_tasks[mclisten.getNumTask()]->listenOutput( mclisten, renders);
   }
   else
   {
      for( int b = 0; b < m_blocksnum; b++)
         for( int t = 0; t < m_blocksdata[b]->getTasksNum(); t++)
			m_blocks[b]->m_tasks[t]->listenOutput( mclisten, renders);
   }
}

int JobAf::v_calcWeight() const
{
//printf("JobAf::calcWeight: '%s' runningtaskscounter=%d\n", name.toUtf8().data(), runningtaskscounter);

   int weight = Job::v_calcWeight();
//printf("JobAf::calcWeight: Job::calcWeight: %d bytes\n", weight);
   weight += sizeof(JobAf) - sizeof( Job);

   if( progress != NULL) progressWeight = progress->calcWeight();
   weight += progressWeight;

   m_logsWeight = calcLogWeight();

   for( int b = 0; b < m_blocksnum; b++)
   {
      weight += m_blocks[b]->calcWeight();
      m_blackListsWeight += m_blocks[b]->blackListWeight();
      m_logsWeight += m_blocks[b]->logsWeight();
   }

   weight += listeners.calcWeight();

   weight += m_blackListsWeight;
   weight += m_logsWeight;

//printf("JobAf::calcWeight: %d bytes ( sizeof JobAf = %d)\n", weight, sizeof(JobAf));
   return weight;
}
