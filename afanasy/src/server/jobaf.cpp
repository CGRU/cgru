#include "jobaf.h"

#include "../include/afanasy.h"

#include "../libafanasy/blockdata.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/jobprogress.h"
#include "../libafanasy/msgqueue.h"

#include "action.h"
#include "afcommon.h"
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
#include "../libafanasy/logger.h"

JobContainer *JobAf::ms_jobs  = NULL;

JobAf::JobAf( JSON & i_object):
	af::Job(),
	AfNodeSolve( this)
{
	initializeValues();
	
	if( false == jsonRead( i_object))
		return;
	
	m_progress = new af::JobProgress( this);
	construct();

	m_serial = AFCommon::getJobSerial();
	// System job never constructed from incoming JSON object and always has zero serial
}

JobAf::JobAf( const std::string & i_store_dir, bool i_system):
	af::Job(),
	AfNodeSolve( this, i_store_dir)
{
	AF_DEBUG << "store dir = " << i_store_dir;
	
	initializeValues();

	// We do not read store for system job
	// as some virtual functions from system job
	// will not work in a base class constructor
	if( i_system ) return;

	readStore();

	// Zero serial means that the job was created serials appeared in the project:
	// ( system job has zero serial )
	if( m_serial == 0 && ( false == i_system ))
	{
		m_serial = AFCommon::getJobSerial();
		store();
	}
}

void JobAf::readStore()
{
	initStoreDirs();

	int size;
	char * data = af::fileRead( getStoreFile(), &size);
	if( NULL == data ) return;

	rapidjson::Document document;
	char * res = af::jsonParseData( document, data, size);
	if( NULL == res )
	{
		delete [] data;
		return;
	}

	jsonRead( document);

	delete [] res;
	delete [] data;

	m_progress = new af::JobProgress( this);

	construct();

	for( int b = 0; b < m_blocks_num; b++)
		if( false == m_blocks[b]->readStoredTasks())
			return;
}

void JobAf::initializeValues()
{
	m_user             = NULL;
	m_blocks           = NULL;
	m_progress         = NULL;
	m_deletion         = false;
	
	m_thumb_changed    = false;
	m_report_changed   = false;
	
	m_logsWeight       = 0;
	m_blackListsWeight = 0;
}

void JobAf::initStoreDirs()
{
	m_store_dir_tasks = getStoreDir() + AFGENERAL::PATH_SEPARATOR + "tasks";
}

void JobAf::construct()
{
	AF_DEBUG << '"' << m_name << "\": from store: " << isFromStore();

	if( NULL != m_blocks )
	{
		AF_ERR << "Already constructed.";
		return;
	}

	if( m_blocks_num < 1 )
	{
		AF_ERR << "invalid number of blocks = " << m_blocks_num;
		return;
	}

	m_blocks = new Block*[m_blocks_num];
	if( NULL == m_blocks )
	{
		AF_ERR << "Can't allocate memory for blocks.";
		return;
	}
	for( int b = 0; b < m_blocks_num; b++) m_blocks[b] = NULL;
	for( int b = 0; b < m_blocks_num; b++)
	{
		m_blocks[b] = v_newBlock(b);
		if( m_blocks[b] == NULL )
		{
			AF_ERR << "Can't create block " << b << " of " << m_blocks_num;
			return;
		}
	}
}

bool JobAf::isValidConstructed() const
{
	std::string err;

	if( isValid( &err))
	{
		if( NULL == m_blocks )
			err += "Job '" + m_name + "' is not constructed.";
	}

	if( err.size())
	{
		err = std::string("Invalid job '") + m_name + "': " + err;
		if( isFromStore())
			AF_ERR << err;
		else
			AFCommon::QueueLogError( err);
		return false;
	}

	AF_DEBUG << '"' << m_name << "\": TRUE; from store: " << isFromStore();

	return true;
}

JobAf::~JobAf()
{
	if( m_blocks )
		{
		for( int b = 0; b < m_blocks_num; b++) if( m_blocks[b]) delete m_blocks[b];
		delete [] m_blocks;
	}
	
	if( m_progress ) delete m_progress;
}

Block * JobAf::v_newBlock( int numBlock)
{
	return new Block( this, m_blocks_data[numBlock], m_progress);
}

void JobAf::setUser( UserAf * i_user)
{
	m_user = i_user;
	for( int b = 0; b < m_blocks_num; b++)
	{
		m_blocks[b]->setUser( i_user);
	}
	m_user_name = i_user->getName();
}

bool JobAf::initialize()
{
	AF_DEBUG << "'" << m_name << "'[" << m_id << "]:";
	
	//
	//	Set job ID to blocks and progress classes:
	m_progress->setJobId( m_id);
	for( int b = 0; b < m_blocks_num; b++)
	{
		m_blocks_data[b]->setJobId( m_id);
	}

	//
	// Store job ( if not stored )
	if( isFromStore() == false )
	{
		setStoreDir( AFCommon::getStoreDirJob( *this));

		initStoreDirs();

		// Write blocks tasks data:
		for( int b = 0; b < m_blocks_num; b++)
			m_blocks[b]->storeTasks();

		std::ostringstream ostr;
		v_jsonWrite( ostr, 0);
		std::string str = ostr.str();
		AFCommon::writeFile( str.c_str(), str.size(), getStoreFile());
	}

	// Create tasks store folder (if does not exists any)
	if(( af::pathIsFolder( m_store_dir_tasks) == false ) && ( af::pathMakePath( m_store_dir_tasks) == false ))
	{
		AFCommon::QueueLogError( std::string("Unable to create tasks store folder:\n") + m_store_dir_tasks);
		return false;
	}

	//
	// Executing pre commands ( if not from database )
	if( isFromStore() == false )
	{
		if( false == m_command_pre.empty())
		{
			AFCommon::executeCmd( m_command_pre);
			appendLog( std::string("Job pre command executed:\n") + m_command_pre);
		}
		for( int b = 0; b < m_blocks_num; b++)
		{
			if( m_blocks_data[b]->hasCmdPre() )
			{
				AFCommon::executeCmd( m_blocks_data[b]->getCmdPre());
				appendLog( std::string("Block[") + m_blocks_data[b]->getName() + "] pre command executed:\n" + m_blocks_data[b]->getCmdPre());
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
	for( int b = 0; b < m_blocks_num; b++)
	{
		int numtasks = m_blocks_data[b]->getTasksNum();
		for( int t = 0; t < numtasks; t++)
		{
			uint32_t taskstate = m_progress->tp[b][t]->state;

			if( taskstate == 0 )
			{
				taskstate = AFJOB::STATE_READY_MASK;
			}
			else if( taskstate & AFJOB::STATE_RUNNING_MASK && ( false == m_blocks_data[b]->isMultiHost()))
			{
				taskstate = taskstate | AFJOB::STATE_WAITRECONNECT_MASK;
				taskstate = taskstate & (~AFJOB::STATE_RUNNING_MASK );
				m_progress->tp[b][t]->time_done = time(NULL);
				m_blocks[b]->m_tasks[t]->v_appendLog(
						"Task was running at server start. Waiting for render reconnect...");
			}

			m_progress->tp[b][t]->state = taskstate;
		}
	}
	
	if(( m_state & AFJOB::STATE_DONE_MASK) == false ) m_state = m_state | AFJOB::STATE_WAITDEP_MASK;
	
	v_refresh( time(NULL), NULL, NULL);
	
	return true;
}

int JobAf::getUid() const { return m_user->getId(); }

void JobAf::deleteNode( RenderContainer * renders, MonitorContainer * monitoring)
{
	if( m_id == AFJOB::SYSJOB_ID )
	{
		AFCommon::QueueLogError("System job can't be zombie");
		return;
	}
	
	if( m_deletion == false )
	{
		m_state = AFJOB::STATE_OFFLINE_MASK;
		lock();
		m_deletion = true;
		
		if( m_custom_data.size() || m_user->getCustomData().size())
		{
			std::vector<std::string> events;
			events.push_back("JOB_DELETED");
			emitEvents(events);
		}
		
		if( getRunningTasksNumber() && (renders != NULL) && (monitoring != NULL))
		{
			restartAllTasks("Job deletion.", renders, monitoring, AFJOB::STATE_RUNNING_MASK);
			if( monitoring ) monitoring->addJobEvent( af::Monitor::EVT_jobs_change, getId(), getUid());
			return;
		}
	}
	if( getRunningTasksNumber() )
	{
		AF_ERR << "runningtaskscounter = " << getRunningTasksNumber();
		return;
	}
	
	if( false == m_command_post.empty())
	{
		SysJob::AddPostCommand( m_command_post, m_blocks_num > 0 ? m_blocks_data[0]->getWDir(): "", m_user_name, m_name);
		appendLog( std::string("Executing job post command:\n") + m_command_post);
	}
	for( int b = 0; b < m_blocks_num; b++)
	{
		if( m_blocks_data[b]->hasCmdPost())
		{
			SysJob::AddPostCommand( m_blocks_data[b]->getCmdPost(), m_blocks_data[b]->getWDir(), m_user_name, m_name);
			appendLog( std::string("Executing block[") + m_blocks_data[b]->getName() + "] post command:\n" + m_blocks_data[b]->getCmdPost());
		}
	}
	
	setZombie();
	
	AFCommon::DBAddJob( this);
	
	if( monitoring ) monitoring->addJobEvent( af::Monitor::EVT_jobs_del, getId(), getUid());
	AFCommon::QueueLog("Deleting a job: " + v_generateInfoString());
	unLock();
}

void JobAf::v_action( Action & i_action)
{
	// If action has blocks ids array - action to for blocks
	if( i_action.data->HasMember("block_ids") || i_action.data->HasMember("block_mask"))
	{
		std::vector<int32_t> block_ids;

		// Try to get block ids from array:
		const JSON & j_block_ids = (*i_action.data)["block_ids"];
		if( j_block_ids.IsArray())
			af::jr_int32vec("block_ids", block_ids, *i_action.data);

		// -1 id is specified = all blocks
		if(( block_ids.size() == 1 ) && ( block_ids[0] == -1 ))
		{
			block_ids.clear();
			for( int b = 0; b < m_blocks_num; b++)
				block_ids.push_back( m_blocks[b]->m_data->getBlockNum());
		}

		// Try to fill ids from mask:
		const JSON & j_block_mask = (*i_action.data)["block_mask"];
		if( j_block_mask.IsString())
		{
			std::string mask;
			af::jr_string("block_mask", mask, *i_action.data);
			af::RegExp re;
			std::string re_err;
			if( false == re.setPattern( mask, &re_err))
			{
				appendLog("Invalid block mask = " + mask + " from " + i_action.author);
				return;
			}
			for( int b = 0; b < m_blocks_num; b++)
				if( re.match( m_blocks[b]->m_data->getName()))
					block_ids.push_back( m_blocks[b]->m_data->getBlockNum());
		}

		if( block_ids.empty())
		{
			appendLog("\"block_ids\" array does not contain any integers or invalid \"block_mask\" from " + i_action.author);
			return;
		}

		bool job_progress_changed = false;
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

		if( job_progress_changed )
			i_action.monitors->addJobEvent( af::Monitor::EVT_jobs_change, getId(), getUid());

		if( i_action.log.size() )
			store();

		return;
	}

	const JSON & operation = (*i_action.data)["operation"];
	if( operation.IsObject())
	{
		std::string type;
		af::jr_string("type", type, operation);
		if( type == "delete")
		{
			if( m_id == AFJOB::SYSJOB_ID )
			{
				appendLog("System job can't be deleted by " + i_action.author);
				return;
			}
			appendLog("Deleted by " + i_action.author);
			m_user->appendLog( "Job \"" + m_name + "\" deleted by " + i_action.author);
			deleteNode( i_action.renders, i_action.monitors);
			i_action.monitors->addJobEvent( af::Monitor::EVT_jobs_del, getId(), getUid());
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
		   restartAllTasks("Job stopped by " + i_action.author, i_action.renders, i_action.monitors, AFJOB::STATE_RUNNING_MASK);
		   m_state = m_state | AFJOB::STATE_OFFLINE_MASK;
		}
		else if( type == "restart_warnings")
		{
			restartAllTasks("Job restart warnings by " + i_action.author,  i_action.renders, i_action.monitors, AFJOB::STATE_WARNING_MASK);
		}
		else if( type == "restart_running")
		{
			restartAllTasks("Job restart running by " + i_action.author,  i_action.renders, i_action.monitors, AFJOB::STATE_RUNNING_MASK);
		}
		else if( type == "restart_skipped")
		{
			restartAllTasks("Job restart skipped by " + i_action.author,  i_action.renders, i_action.monitors, AFJOB::STATE_SKIPPED_MASK);
		}
		else if( type == "restart_done")
		{
			restartAllTasks("Job restart done by " + i_action.author,  i_action.renders, i_action.monitors, AFJOB::STATE_DONE_MASK);
		}
		else if( type == "reset_error_hosts")
		{
			for( int b = 0; b < m_blocks_num; b++)
				m_blocks[b]->action( i_action);
		}
		else if( type == "restart")
		{
			restartAllTasks("Job restarted by " + i_action.author,  i_action.renders, i_action.monitors);
			checkDepends();
			m_time_started = 0;
		}
		else if( type == "restart_errors")
		{
			restartAllTasks("Job errors restarted by " + i_action.author,  i_action.renders, i_action.monitors, AFJOB::STATE_ERROR_MASK);
		}
		else if( type == "restart_pause")
		{
			restartAllTasks("Job restarted ( and paused ) by " + i_action.author,  i_action.renders, i_action.monitors);
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
		i_action.monitors->addJobEvent( af::Monitor::EVT_jobs_change, getId(), getUid());
		store();
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
		UserAf * user = i_action.users->getUser( m_user_name);
		if( user == NULL )
		{
			return;
		}
		
		i_action.monitors->addEvent(    af::Monitor::EVT_users_change, m_user->getId());
		i_action.monitors->addJobEvent( af::Monitor::EVT_jobs_del, getId(), m_user->getId());
		
		m_user->removeJob( this);
		user->addJob( this);
		
		i_action.monitors->addEvent(    af::Monitor::EVT_users_change, m_user->getId());
		i_action.monitors->addJobEvent( af::Monitor::EVT_jobs_add, getId(), m_user->getId());
		
		store();

		return;
	}

	if( i_action.log.size() )
	{
		store();
		i_action.monitors->addJobEvent( af::Monitor::EVT_jobs_change, getId(), getUid());
	}
}

void JobAf::v_priorityChanged( MonitorContainer * i_monitoring)
{
	m_user->jobPriorityChanged( this, i_monitoring);
}

void JobAf::setUserListOrder( int index, bool updateDtabase)
{
	int old_index = m_user_list_order;
	m_user_list_order = index;

	// user list order == -1 at job creation, and we do not store here in any case:
	if(( old_index != -1 ) && ( index != old_index ) && updateDtabase )
		store();
}

void JobAf::checkDepends()
{
	m_state = m_state & (~AFJOB::STATE_WAITDEP_MASK);
	
	bool depend_local = false;
	bool depend_global = false;
	
	// check global depends:
	if( hasDependMaskGlobal())
	{
		JobContainerIt jobsIt( ms_jobs);
		for( Job *job = jobsIt.job(); job != NULL; jobsIt.next(), job = jobsIt.job())
		{
			if( job == this ) continue;
			if(( job->isDone() == false ) && ( checkDependMaskGlobal( job->getName()) ))
			{
				depend_global = true;
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
			if(( ((JobAf*)job)->isDone() == false ) && ( checkDependMask( ((JobAf*)job)->getName()) ))
			{
				depend_local = true;
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
				appendLog( std::string("Block[") + m_blocks_data[block]->getName() + "] appears second time while job generating a task.\nJob has a recursive blocks tasks dependence.");
				m_state = m_state | AFJOB::STATE_OFFLINE_MASK;
				if( monitoring ) monitoring->addJobEvent( af::Monitor::EVT_jobs_change, getId(), getUid());
				return NULL;
			}
			bIt++;
		}
		blocksIds->push_back( block);
	}
	//
	
	if( false == ( m_blocks_data[block]->getState() & AFJOB::STATE_READY_MASK ) ) return NULL;
	if( task >= m_blocks_data[block]->getTasksNum() )
	{
		AF_ERR << "block[" << block << "] '" << m_blocks_data[block]->getName()
		       << "' : " << task << " >= number of tasks = " << m_blocks_data[block]->getTasksNum();
		return NULL;
	}
	if( false == ( m_progress->tp[block][task]->state & AFJOB::STATE_READY_MASK) ) return NULL;
	
	if( false == m_blocks[block]->canRunOn( render)) return NULL;
	
	if( m_blocks[block]->m_tasks[task]->avoidHostsCheck( render->getName())) return NULL;
	
	//
	// Check block tasks dependence: Get tasks depend mask, if any exists:
	if( m_blocks_data[block]->hasTasksDependMask() )
	{
		bool dependsnotdone = false;
		std::list<int> blocksIdsBranch;
		if( blocksIds)
		{
			blocksIdsBranch = *blocksIds;
			blocksIds = &blocksIdsBranch;
		}
		
		for( int b = 0; b < m_blocks_num; b++)
		{
			if( b == block ) continue;
			
			//         if( blocksdata[block]->checkTasksDependMask( blocksdata[b]->getName()) == false ) continue;
			if( m_blocks[block]->tasksDependsOn( b) == false )
				continue;
			
			long long firstdependframe, lastdependframe;
			int firstdependtask, lastdependtask;
			m_blocks_data[block]->genNumbers( firstdependframe, lastdependframe, task);
			if( m_blocks_data[block]->isNumeric() && m_blocks_data[b]->isNotNumeric())
			{
				firstdependframe -= m_blocks_data[block]->getFrameFirst();
				lastdependframe  -= m_blocks_data[block]->getFrameFirst();
			}
			else if( m_blocks_data[block]->isNotNumeric() && m_blocks_data[b]->isNumeric())
			{
				firstdependframe += m_blocks_data[b]->getFrameFirst();
				lastdependframe  += m_blocks_data[b]->getFrameFirst();
			}
			if( m_blocks_data[b]->getFramePerTask() < 0 ) lastdependframe++; // For several frames in task
			
			bool inValidRange;
			firstdependtask = m_blocks_data[b]->calcTaskNumber( firstdependframe, inValidRange);
			lastdependtask  = m_blocks_data[b]->calcTaskNumber(  lastdependframe, inValidRange);
			if( inValidRange )
				if( m_blocks_data[b]->getFramePerTask() < 0 )
					lastdependtask--;
			
			for( int t = firstdependtask; t <= lastdependtask; t++)
			{
				// Task is done, so depend is satisfied:
				if( m_progress->tp[b][t]->state & AFJOB::STATE_DONE_MASK )
					continue;
				
				// Check subframe depend, is depend task is running:
				if( m_blocks_data[b]->isDependSubTask() && ( m_progress->tp[b][t]->state & AFJOB::STATE_RUNNING_MASK ))
				{
					long long f_start_dep, f_end_dep;
					m_blocks_data[b]->genNumbers( f_start_dep, f_end_dep, t);
					long long frame_run = f_start_dep + m_progress->tp[b][t]->frame;
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
				
				if( task_ptr ) return task_ptr;
				if( dependsnotdone == false ) dependsnotdone = true;
			}
		}
		if( dependsnotdone ) return NULL;
	}
	
	af::TaskExec * task_exec = m_blocks_data[block]->genTask( task);
	
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
	if( m_blocks_num < 1)
	{
		AF_ERR << "job has no blocks";
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
	// Check Render Nimby:
	if( false == isIgnoreNimbyFlag())
	{
		if( i_render->isNIMBY())
			return false;

		if( i_render->isNimby())
			if( i_render->getUserName() != m_user_name )
				return false;
	}

	// Check Render Paused:
	if( false == isIgnorePausedFlag())
	{
		if( i_render->isPaused())
			return false;
	}

	// check maximum running tasks per host:
	if(( m_max_running_tasks_per_host  > 0 ) && ( getRenderCounts(i_render) >= m_max_running_tasks_per_host ))
	{
		return false;
	}
	
	// check at least one block can run on render
	bool blockCanRunOn = false;
	for( int b = 0; b < m_blocks_num; b++)
	{
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

RenderAf * JobAf::v_solve( std::list<RenderAf*> & i_renders_list, MonitorContainer * i_monitoring)
{
	for( std::list<RenderAf*>::iterator rIt = i_renders_list.begin(); rIt != i_renders_list.end(); rIt++)
	{
		if( solveOnRender( *rIt, i_monitoring))
			return *rIt;
	}

	return NULL;
}

bool JobAf::solveOnRender( RenderAf * i_render, MonitorContainer * i_monitoring)
{
	// Prepare for the new solving (reset previous solvind stored data):
	for( int b = 0; b < m_blocks_num; b++)
	{
		int numtasks = m_blocks_data[b]->getTasksNum();
		for( int t = 0; t < numtasks; t++)
		{
			// Needed for recursion function, to not to try to solve the same task again
			m_blocks[b]->m_tasks[t]->m_solved = false;
			
			// Needed to store tasks that was tried, for nonsequential case:
			m_progress->tp[b][t]->setNotSolved();
		}
	}
	
	for( int b = 0; b < m_blocks_num; b++)
	{
		if( false == ( m_blocks_data[b]->getState() & AFJOB::STATE_READY_MASK )) continue;
		
		int task_num = m_blocks_data[b]->getReadyTaskNumber( m_progress->tp[b], m_flags, i_render);
		
		if( task_num == AFJOB::TASK_NUM_NO_TASK )
		{
			// Block has no ready tasks
			continue;
		}
		
		if( task_num == AFJOB::TASK_NUM_NO_SEQUENTIAL )
		{
			// All non sequential tasks solved, and job is PPA:
			m_state = m_state | AFJOB::STATE_PPAPPROVAL_MASK;
			continue;
		}
		
		if( m_state & AFJOB::STATE_PPAPPROVAL_MASK )
			m_state = m_state & ( ~ AFJOB::STATE_PPAPPROVAL_MASK );
		
		if( task_num < 0 )
		{
			AF_ERR << "Invalid task number returned from af::BlockData::getReadyTaskNumber()";
			continue;
		}
		
		std::list<int> blocksIds;
		af::TaskExec *task_exec = genTask( i_render, b, task_num, &blocksIds, i_monitoring);
		
		// Job may became paused, if recursion during task generation detected:
		if( m_state & AFJOB::STATE_OFFLINE_MASK )
		{
			if( task_exec ) delete task_exec;
			return false;
		}
		
		// Check if render is online
		// It can be solved with offline render to check whether to WOL wake it
		if( task_exec && i_render->isOffline())
		{
			delete task_exec;
			return true;
		}
		
		// No task was generated:
		if( NULL == task_exec ) continue;
		
		// Job successfully solved (produced a task)
		task_exec->setJobName( m_name);
		task_exec->setUserName( m_user_name);
		
		// Job was not able to start a task.
		// This should not happen.
		// This is some error situation (probably system job), see server log.
		if( false == m_blocks[task_exec->getBlockNum()]->v_startTask( task_exec, i_render, i_monitoring))
		{
			delete task_exec;
			continue;
		}

		m_blocks[task_exec->getBlockNum()]->m_data->setTimeStarted( time(NULL) );
		
		// If job was not started it became started
		if( m_time_started == 0 )
		{
			m_time_started = time(NULL);
			appendLog("Started.");
			store();
		}

		// Set RUNNING state if it was not:
		if( false == ( m_state & AFJOB::STATE_RUNNING_MASK ))
			m_state |= AFJOB::STATE_RUNNING_MASK;
		
		return true;
	}
	return false;
}

void JobAf::v_updateTaskState( const af::MCTaskUp& taskup, RenderContainer * renders, MonitorContainer * monitoring)
{
	if( false == checkBlockTaskNumbers( taskup.getNumBlock(), taskup.getNumTask(), "updateTaskState")) return;
	bool errorHost = false;
	m_blocks[taskup.getNumBlock()]->m_tasks[taskup.getNumTask()]->v_updateState( taskup, renders, monitoring, errorHost);
	if( errorHost) m_blocks[taskup.getNumBlock()]->v_errorHostsAppend( taskup.getNumTask(), taskup.getClientId(), renders);

	std::string new_report = taskup.getReport();
	if( new_report.size() && ( new_report != m_report ))
	{
		m_report = new_report;
		m_report_changed = true;
	}
}

void JobAf::reconnectTask(af::TaskExec *i_taskexec, RenderAf & i_render, MonitorContainer * i_monitoring)
{
	int b = i_taskexec->getBlockNum();
	int t = i_taskexec->getTaskNum();
	if( false == checkBlockTaskNumbers( b, t, "reconnectTask"))
	{
		delete i_taskexec;
		return;
	}
	m_blocks[b]->reconnectTask( i_taskexec, i_render, i_monitoring);
}

void JobAf::v_refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
	RenderContainer * renders = (RenderContainer*)pointer;
	
	if( m_deletion )
	{
		for( int b = 0; b < m_blocks_num; b++)
			m_blocks[b]->v_refresh( currentTime, renders, monitoring);
		if( getRunningTasksNumber() == 0 ) deleteNode( NULL, monitoring);
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
		if( m_time_wait > currentTime ) m_state = m_state | AFJOB::STATE_WAITTIME_MASK;
		else m_state = m_state & (~AFJOB::STATE_WAITTIME_MASK);
		bool nowWaining = m_state & AFJOB::STATE_WAITTIME_MASK;
		if( wasWaiting != nowWaining) jobchanged = af::Monitor::EVT_jobs_change;
	}

	//
	// Update blocks (blocks will uptate its tasks):
	for( int b = 0; b < m_blocks_num; b++)
		if( m_blocks[b]->v_refresh( currentTime, renders, monitoring))
			jobchanged = af::Monitor::EVT_jobs_change;
	// Check block depends after all block refreshed,
	// as states can be changed during refresh.
	// ( some block can be DONE after refresh )
	for( int b = 0; b < m_blocks_num; b++)
		if( m_blocks[b]->checkDepends( monitoring))
			jobchanged = af::Monitor::EVT_jobs_change;
	
	//
	// job state calculation
	m_state = m_state |   AFJOB::STATE_DONE_MASK;
	m_state = m_state & (~AFJOB::STATE_RUNNING_MASK);
	m_state = m_state & (~AFJOB::STATE_WARNING_MASK);
	m_state = m_state & (~AFJOB::STATE_ERROR_MASK);
	m_state = m_state & (~AFJOB::STATE_READY_MASK);
	m_state = m_state & (~AFJOB::STATE_SKIPPED_MASK);
	
	for( int b = 0; b < m_blocks_num; b++)
	{
		uint32_t state_block = m_blocks_data[b]->getState();
		m_state  = m_state | (state_block &   AFJOB::STATE_RUNNING_MASK   );
		m_state  = m_state | (state_block &   AFJOB::STATE_WARNING_MASK   );
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
			jobchanged = af::Monitor::EVT_jobs_change;
			store();
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
		for( int b = 0; b < m_blocks_num; b++ )
		{
			if( m_blocks_data[b]->getProgressTasksDone() > 0 )
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
		jobchanged = af::Monitor::EVT_jobs_change;
		
		// If it is no job monitoring, job just came to server and it is first it refresh,
		// so no change event and database storing needed
		if( monitoring )
			store();
		
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
				
				emitEvents(events);
			}
	}
	
	if( m_thumb_changed || m_report_changed )
	{
		jobchanged = af::Monitor::EVT_jobs_change;
		m_thumb_changed = false;
		m_report_changed = false;
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
			deleteNode( renders, monitoring);
			jobchanged = af::Monitor::EVT_jobs_del, getId(), getUid();
		}
	}
	
	if(( monitoring ) &&  ( jobchanged )) monitoring->addJobEvent( jobchanged, getId(), getUid());
	
	// Update solving parameters:
	v_calcNeed();
}

void JobAf::emitEvents(std::vector<std::string> events)
{
	// Processing command for system job if some events happened:
	if( events.empty())
		return;
	
	std::ostringstream str;
	str << "{";

	str << "\n\"user\":";
	m_user->v_jsonWrite( str, af::Msg::TUsersList);

	str << ",\n\"job\":";
	v_jsonWrite( str, af::Msg::TJobsList);

	str << ",\n\"events\":[";

	for( int i = 0; i < events.size(); i++ )
	{
		if( i ) str << ',';
		str << '"' << events[i] << '"';
	}
	str << "]\n}";
	
	SysJob::AddEventCommand( str.str(),
		"", // working directory - no matter
		m_user_name, m_name, events[0]);
}

void JobAf::v_calcNeed()
{
	// Need calculation based on running tasks number
	if( af::Environment::getSolvingUseCapacity())
		calcNeedResouces( getRunningCapacityTotal());
	else
		calcNeedResouces( getRunningTasksNumber());
}
void JobAf::restartAllTasks( const std::string & i_message, RenderContainer * i_renders, MonitorContainer * i_monitoring, uint32_t i_state)
{
	for( int b = 0; b < m_blocks_num; b++)
	{
		int numtasks = m_blocks_data[b]->getTasksNum();
		for( int t = 0; t < numtasks; t++)
		{
			m_blocks[b]->m_tasks[t]->restart( i_message, i_renders, i_monitoring, i_state);
		}
	}

	v_refresh( time(NULL), i_renders, i_monitoring);
}

void JobAf::writeProgress( af::Msg &msg)
{
	msg.set( af::Msg::TJobProgress, m_progress);
}

af::Msg * JobAf::writeProgress( bool json)
{
	af::Msg * msg = new af::Msg(); 
	if( json )
	{
		std::ostringstream stream;
		m_progress->jsonWrite( stream);
		std::string string = stream.str();
		msg->setData( string.size(), string.c_str(), af::Msg::TJSON);
	}
	else
	{
		msg->set( af::Msg::TJobProgress, m_progress);
	}

	return msg;
}

af::Msg * JobAf::writeBlocks( std::vector<int32_t> i_block_ids, std::vector<std::string> i_modes, bool i_binary) const
{
	if( i_block_ids.size() != i_modes.size())
	{
		AF_ERR << "i_block_ids.size() != i_modes.size(): " << i_block_ids.size() << " != " << i_modes.size();
		return NULL;
	}

	if( i_binary )
	{
		af::MCAfNodes mcblocks;
		for( int b = 0; b < i_block_ids.size(); b++)
		mcblocks.addNode( m_blocks_data[i_block_ids[b]]);
		return new af::Msg( af::BlockData::DataModeFromString( i_modes[0]), &mcblocks);
	}

	std::ostringstream str;
	str << "{\"blocks\":[\n";
	for( int b = 0; b < i_block_ids.size(); b++)
	{
		if( b > 0 ) str << ",";
		if( i_block_ids[b] >= m_blocks_num )
		{
			AF_ERR << "i_block_ids[b] >= m_blocks_num: " << i_block_ids[b] << " >= " << m_blocks_num;
			return NULL;
		}
		m_blocks_data[i_block_ids[b]]->jsonWrite( str, i_modes[b]);
	}
	str << "\n]}";

	return af::jsonMsg( str);
}

af::Msg * JobAf::writeTask( int i_b, int i_t, const std::string & i_mode, bool i_binary) const
{
	std::ostringstream str;
	str << "{";

	if( false == checkBlockTaskNumbers( i_b, i_t, "writeTask"))
	{
		return af::jsonMsgError("Invalid block/task number.");
	}

	af::MCTask mctask( m_id, i_b, i_t);
	fillTaskNames( mctask);

	if( i_mode == "info" )
	{
		af::TaskExec * exec = generateTask( i_b, i_t);
		if( exec )
		{
			mctask.setExec( exec);
			return mctask.generateMessage( i_binary);
		}
	}
	else if( i_mode == "log" )
	{
		mctask.setLog( getTaskLog( i_b, i_t));
		return mctask.generateMessage( i_binary);
	}
	else if( i_mode == "error_hosts")
	{
		std::list<std::string> list;
		m_blocks[i_b]->m_tasks[i_t]->getErrorHostsList( list);
		mctask.setErrorHosts( list);
		return mctask.generateMessage( i_binary);
	}
	else if( i_mode == "files" )
	{
		if( i_binary )
			return m_blocks[i_b]->m_tasks[i_t]->getStoredFiles();
		else
			m_blocks[i_b]->m_tasks[i_t]->getStoredFiles( str);
	}
	else if( i_mode == "output")
	{
		// "output" is processed in threadProcessJSON(),
		// as file reading or render update waiting can be needed.
		AF_ERR << "Mode is \"output\".";
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
	return m_blocks[block]->m_tasks[task]->genExec();
}
af::Msg * JobAf::writeErrorHosts( bool i_binary) const
{
	std::list<std::string> list;
	for( int block = 0; block < m_blocks_num; block++)
		m_blocks[block]->v_getErrorHostsList( list);

	if( false == i_binary )
		return af::jsonMsg("error_hosts", m_name, list);

	std::string str("Job \"");
	str += m_name + "\" error hosts:\n";
	str += af::strJoin( list, "\n");

	af::Msg * msg = new af::Msg;
	msg->setString( str);
	return msg;
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

bool JobAf::checkBlockTaskNumbers( int i_b, int i_t, const char * o_str) const
{
	if(( i_b >= m_blocks_num) || ( i_b < 0 ))
	{
		if( o_str )
			AF_ERR << o_str << ": numblock >= blocksnum ( " << i_b << " >= " << m_blocks_num << " )";
		else
			AF_ERR << "numblock >= blocksnum ( " << i_b << " >= " << m_blocks_num << " )";
		return false;
	}

	if(( i_t >= m_blocks_data[i_b]->getTasksNum()) || ( i_t < 0 ))
	{
		if( o_str )
			AF_ERR << o_str << ": numtask >= numTasks ( " << i_t << " >= " << m_blocks_data[i_b]->getTasksNum() << " )";
		else
			AF_ERR << "numtask >= numTasks ( " << i_t << " >= " << m_blocks_data[i_b]->getTasksNum() << " )";
		return false;
	}

	return true;
}
void JobAf::v_getTaskOutput( af::MCTask & io_mctask, std::string & o_error) const
{
	int b = io_mctask.getBlockNum();
	int t = io_mctask.getTaskNum();

	if( false == checkBlockTaskNumbers( b, t, "getTaskOutput"))
	{
		o_error = "Invalid block and task numbers";
		return;
	}

	fillTaskNames( io_mctask);

	m_blocks[b]->m_tasks[t]->getOutput( io_mctask, o_error);
}

void JobAf::fillTaskNames( af::MCTask & o_mctask) const
{
	int b = o_mctask.getBlockNum();
	int t = o_mctask.getTaskNum();
	o_mctask.m_job_name   = getName();
	o_mctask.m_block_name = m_blocks[b]->m_data->getName();
	o_mctask.m_task_name  = m_blocks[b]->m_data->genTaskName(t);
	o_mctask.m_service    = m_blocks[b]->m_data->getService();
	o_mctask.m_parser     = m_blocks[b]->m_data->getParser();
	o_mctask.m_progress   = *(m_progress->tp[b][t]);
}

void JobAf::listenOutput( RenderContainer * i_renders, bool i_subscribe, int i_block, int i_task)
{
	if(( i_block >= 0 ) && ( i_task >= 0 ))
	{
		if( false == checkBlockTaskNumbers( i_block, i_task, "listenOutput"))
			return;

		m_blocks[i_block]->m_tasks[i_task]->listenOutput( i_renders, i_subscribe);
	}
	else
	{
		for( int b = 0; b < m_blocks_num; b++)
			for( int t = 0; t < m_blocks_data[b]->getTasksNum(); t++)
				m_blocks[b]->m_tasks[t]->listenOutput( i_renders, i_subscribe);
	}
}

void JobAf::setThumbnail( const std::string & i_path, int i_size, const char * i_data)
{
	if( m_thumb_path == i_path ) return;
	if( i_size == 0 ) return;
	if( i_data == NULL ) return;

	m_thumb_size = i_size;
	m_thumb_path = i_path;
	if( m_thumb_data ) delete [] m_thumb_data;
	m_thumb_data = new char[m_thumb_size];
	memcpy( m_thumb_data, i_data, m_thumb_size);

	m_thumb_changed = true;
}

af::Msg * JobAf::writeThumbnail( bool i_binary)
{
	if( i_binary )
	{
		af::MCTaskUp taskup( -1, getId(), -1, -1);
		taskup.addFile( m_thumb_path, m_thumb_data, m_thumb_size);
		return new af::Msg( af::Msg::TTaskFiles, &taskup);
	}

	return NULL;
}

int JobAf::v_calcWeight() const
{
	int weight = Job::v_calcWeight();
	weight += sizeof(JobAf) - sizeof( Job);
	
	if( m_progress != NULL) progressWeight = m_progress->calcWeight();
	weight += progressWeight;
	
	m_logsWeight = calcLogWeight();
	
	for( int b = 0; b < m_blocks_num; b++)
	{
		weight += m_blocks[b]->calcWeight();
		m_blackListsWeight += m_blocks[b]->blackListWeight();
		m_logsWeight += m_blocks[b]->logsWeight();
	}
	
	weight += m_blackListsWeight;
	weight += m_logsWeight;
	
	return weight;
}
