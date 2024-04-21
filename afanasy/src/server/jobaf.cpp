/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

/*
	This is a server side of an Afanasy job.
*/
#include "jobaf.h"

#include "../include/afanasy.h"

#include "../libafanasy/blockdata.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/jobprogress.h"
#include "../libafanasy/msgqueue.h"

#include "action.h"
#include "afcommon.h"
#include "block.h"
#include "branchescontainer.h"
#include "branchsrv.h"
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

	if (false == isValid())
		return;

	m_progress = new af::JobProgress( this);
	construct();

	m_serial = AFCommon::getJobSerial();
	// System job never constructed from incoming JSON object and always has zero serial

	for (int b = 0; b < m_blocks_num; b++)
		m_blocks[b]->constructDependTasks();
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

	for (int b = 0; b < m_blocks_num; b++)
		m_blocks[b]->constructDependTasks();
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
	m_force_refresh    = true;

	m_branch_srv       = NULL;
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

void JobAf::construct(int alreadyConstructed)
{
	AF_DEBUG << '"' << m_name << "\": from store: " << isFromStore();

	if( NULL != m_blocks && alreadyConstructed == 0 )
	{
		AF_ERR << "Already constructed.";
		return;
	}

	if( m_blocks_num < 1 )
	{
		AF_ERR << "invalid number of blocks = " << m_blocks_num;
		return;
	}

	Block ** old_blocks = m_blocks;

	m_blocks = new Block*[m_blocks_num];
	if( NULL == m_blocks )
	{
		AF_ERR << "Can't allocate memory for blocks.";
		return;
	}
	for( int b = 0; b < m_blocks_num; b++)
		m_blocks[b] = b < alreadyConstructed ? old_blocks[b] : NULL;
	if( NULL != old_blocks)
		delete [] old_blocks;
	for( int b = alreadyConstructed; b < m_blocks_num; b++)
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
	bool valid = true;
	std::string err;

	if ((NULL == m_blocks) || (NULL == m_progress))
	{
		err += "Is not constructed.";
		valid = false;
	}

	if (valid)
	{
		valid = isValid(&err);
	}

	if (false == valid)
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

void JobAf::setBranch(BranchSrv * i_branch)
{
	m_branch_srv = i_branch;
	m_branch = m_branch_srv->getName();
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
		// Process event
		if( m_id != AFJOB::SYSJOB_ID ) // skip system job
		{
			if (hasCustomData() || m_user->hasCustomData())
			{
				std::vector<std::string> events;
				events.push_back("JOB_CREATED");
				emitEvents(events);
			}
		}

		appendLog("Initialized.");
	}
	else
	{
		appendLog("Initialized from database.");
	}

	checkStates();

	v_refresh( time(NULL), NULL, NULL);

	return true;
}

void JobAf::checkStates()
{
	// This function is called on a job initialization,
	// When a new job created, or from database on server restart.
	// Also it called on a block(s) appending.

	for( int b = 0; b < m_blocks_num; b++)
	{
		int numtasks = m_blocks_data[b]->getTasksNum();
		for( int t = 0; t < numtasks; t++)
		{
			uint32_t taskstate = m_progress->tp[b][t]->state;

			if (taskstate == 0)
			{
				// This is a new job, not from database
				if (m_blocks_data[b]->isSuspendingNewTasks())
					taskstate = AFJOB::STATE_SUSPENDED_MASK;
				else
					taskstate = AFJOB::STATE_READY_MASK;
			}
			else if (taskstate == AFJOB::STATE_WARNING_MASK)
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

	// If job is not done, just set WAITDEP state to not make it to run before a refresh.
	if(( m_state & AFJOB::STATE_DONE_MASK) == false ) m_state = m_state | AFJOB::STATE_WAITDEP_MASK;
}

void JobAf::checkStatesOnAppend()
{
	for( int b = 0; b < m_blocks_num; b++)
	{
		int numtasks = m_blocks_data[b]->getTasksNum();
		for( int t = 0; t < numtasks; t++)
		{
			uint32_t taskstate = m_progress->tp[b][t]->state;

			if( taskstate == 0 )
			{
				if (m_blocks_data[b]->isSuspendingNewTasks())
					taskstate = AFJOB::STATE_SUSPENDED_MASK;
				else
					taskstate = AFJOB::STATE_READY_MASK;
				m_progress->tp[b][t]->state = taskstate;
			}
		}
	}

	if(( m_state & AFJOB::STATE_DONE_MASK) == false ) m_state = m_state | AFJOB::STATE_WAITDEP_MASK;
}

int JobAf::getUid() const { return m_user->getId(); }

void JobAf::deleteNode( RenderContainer * renders, MonitorContainer * monitoring)
{
	if (m_id == AFJOB::SYSJOB_ID)
	{
		AF_ERR << "An attempt to delete a system job.";
		return;
	}
	
	if (false == m_deletion)
	{
		m_state |= AFJOB::STATE_OFFLINE_MASK;
		m_state &= ~AFJOB::STATE_READY_MASK;

		lock();
		m_deletion = true;
		
		if (hasCustomData() || m_user->hasCustomData())
		{
			std::vector<std::string> events;
			events.push_back("JOB_DELETED");
			emitEvents(events);
		}
		
		if (getRunningTasksNum() && (renders != NULL) && (monitoring != NULL))
		{
			restartAllTasks("Job deletion.", renders, monitoring, AFJOB::STATE_RUNNING_MASK);
			if( monitoring ) monitoring->addJobEvent( af::Monitor::EVT_jobs_change, getId(), getUid());
			return;
		}
	}

	// Wait for all running tasks stop:
	if (getRunningTasksNum())
	{
		return;
	}

	// Process job post command:
	if (false == m_command_post.empty())
	{
		SysJob::AddPostCommand( m_command_post, m_blocks_num > 0 ? m_blocks_data[0]->getWDir(): "", m_user_name, m_name);
		appendLog( std::string("Executing job post command:\n") + m_command_post);
	}
	// Process blocks post commands:
	for (int b = 0; b < m_blocks_num; b++)
	{
		if (m_blocks_data[b]->hasCmdPost())
		{
			SysJob::AddPostCommand( m_blocks_data[b]->getCmdPost(), m_blocks_data[b]->getWDir(), m_user_name, m_name);
			appendLog( std::string("Executing block[") + m_blocks_data[b]->getName() + "] post command:\n" + m_blocks_data[b]->getCmdPost());
		}
	}
	
	setZombie();
	
	AFCommon::DBAddJob( this);

	m_branch_srv->removeJob(this, m_user);
	
	if (monitoring)
	{
		monitoring->addJobEvent(af::Monitor::EVT_jobs_del, getId(), getUid());
		monitoring->addEvent(af::Monitor::EVT_branches_change, m_branch_srv->getId());
	}

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

		bool job_changed = false;
		for( int b = 0; b < block_ids.size(); b++)
		{
			if(( block_ids[b] >= getBlocksNum()) || ( block_ids[b] < 0 ))
			{
				appendLog("Invalid block number = " + af::itos(block_ids[b]) + " " + i_action.author);
				continue;
			}
			if( m_blocks[block_ids[b]]->action( i_action))
				job_changed = true;
		}

		if (job_changed)
		{
			i_action.monitors->addJobEvent( af::Monitor::EVT_jobs_change, getId(), getUid());
			m_force_refresh = true;
			store();
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
			if( m_id == AFJOB::SYSJOB_ID )
			{
				appendLog("System job can't be deleted by " + i_action.author);
				return;
			}
			appendLog("Deleted by " + i_action.author);
			m_user->appendLog( "Job \"" + m_name + "\" deleted by " + i_action.author);
			deleteNode( i_action.renders, i_action.monitors);
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
		else if(type == "reset_trying_next_tasks")
		{
			resetTryTasksNext();
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
		else if( type == "append_blocks")
		{
			appendBlocks(i_action, operation);
			return;
		}
		else
		{
			appendLog("Unknown operation \"" + type + "\" by " + i_action.author);
			i_action.answerError("Unknown operation: " + type);
			return;
		}
		appendLog("Operation \"" + type + "\" by " + i_action.author);
		i_action.monitors->addJobEvent( af::Monitor::EVT_jobs_change, getId(), getUid());
		m_force_refresh = true;
		store();
		return;
	}

	// Store some parameters before read, to check whether it changed
	const std::string _user_name = m_user_name;
	const std::string _branch = m_branch;

	const JSON & params = (*i_action.data)["params"];
	if( params.IsObject())
		jsonRead( params, &i_action.log);

	if( m_user_name != _user_name )
	{
		// User name was changed

		UserAf * user = i_action.users->getUser( m_user_name);
		if( user == NULL )
		{
			i_action.answerError("User does not exist: " + m_user_name);
			m_user_name = _user_name;
			return;
		}

		i_action.monitors->addEvent(    af::Monitor::EVT_users_change, m_user->getId());
		i_action.monitors->addJobEvent( af::Monitor::EVT_jobs_del, getId(), m_user->getId());

		m_branch_srv->changeJobUser(m_user, this, user);
		m_user->removeJob( this);
		user->addJob( this);  //< UserAf::addJob() updates JobAf::m_user

		i_action.monitors->addEvent(    af::Monitor::EVT_users_change, m_user->getId());
		i_action.monitors->addJobEvent( af::Monitor::EVT_jobs_add, getId(), m_user->getId());

		store();

		return;
	}

	if (m_branch != _branch)
	{
		// Branch was changed

		BranchSrv * new_branch_srv = i_action.branches->getBranch(m_branch);
		if (new_branch_srv == NULL)
		{
			i_action.answerError("New job branch not found: " + m_branch);
			m_branch = _branch;
			return;
		}

		m_branch_srv->removeJob(this, m_user);
		i_action.monitors->addEvent(af::Monitor::EVT_branches_change, m_branch_srv->getId());

		new_branch_srv->addJob(this, m_user);
		i_action.monitors->addEvent(af::Monitor::EVT_branches_change, m_branch_srv->getId());
	}

	if (i_action.log.size())
	{
		// Not empty log means some parameter change:
		store();
		m_force_refresh = true;
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
	
	return true;
}

void JobAf::addSolveCounts(MonitorContainer * i_monitoring, af::TaskExec * i_exec, RenderAf * i_render)
{
	m_user->addSolveCounts(i_monitoring, i_exec, i_render);
	m_branch_srv->addSolveCounts(i_monitoring, i_exec, i_render, m_user);

	AfNodeSolve::addSolveCounts(i_exec, i_render);
	i_monitoring->addJobEvent(af::Monitor::EVT_jobs_change, getId(), m_user->getId());
}

void JobAf::remSolveCounts(MonitorContainer * i_monitoring, af::TaskExec * i_exec, RenderAf * i_render)
{
	m_user->remSolveCounts(i_monitoring, i_exec, i_render);
	m_branch_srv->remSolveCounts(i_monitoring, i_exec, i_render, m_user);

	AfNodeSolve::remSolveCounts(i_exec, i_render);
	i_monitoring->addJobEvent(af::Monitor::EVT_jobs_change, getId(), m_user->getId());
}

bool JobAf::checkTryTasksNext()
{
	if (m_try_this_tasks_num.size() == m_try_this_blocks_num.size())
	{
		for (int i = 0; i < m_try_this_tasks_num.size(); i++)
		{
			int t = m_try_this_tasks_num[i];
			int b = m_try_this_blocks_num[i];

			if (b < 0)
			{
				AF_ERR << "Tasks to try next has a negative block number.";
				resetTryTasksNext();
				return false;
			}

			if (t < 0)
			{
				AF_ERR << "Tasks to try next has a negative task number.";
				resetTryTasksNext();
				return false;
			}

			if (b >= m_blocks_num)
			{
				AF_ERR << "Tasks to try next has an invalid block number.";
				resetTryTasksNext();
				return false;
			}

			if (t >= m_blocks_data[b]->getTasksNum())
			{
				AF_ERR << "Tasks to try next has an invalid task number.";
				resetTryTasksNext();
				return false;
			}
		}

		return true;
	}

	AF_ERR << "Try next tasks num and blocks num mismatch: "
		<< m_try_this_tasks_num.size() << " != " << m_try_this_blocks_num.size();

	resetTryTasksNext();

	return false;
}

void JobAf::resetTryTasksNext()
{
	for (int b = 0; b < m_blocks_num; b++)
		for (int t = 0; t < m_blocks_data[b]->getTasksNum(); t++)
			m_progress->tp[b][t]->state &= (~AFJOB::STATE_TRYTHISTASKNEXT_MASK);

	m_try_this_tasks_num.clear();
	m_try_this_blocks_num.clear();
}

void JobAf::tryTaskNext(bool i_append, int i_block_num, int i_task_num)
{
	if (false == checkTryTasksNext())
		return;

	std::vector<int32_t>::iterator tIt = m_try_this_tasks_num.begin();
	std::vector<int32_t>::iterator bIt = m_try_this_blocks_num.begin();

	while (tIt != m_try_this_tasks_num.end())
	{
		if ((*tIt == i_task_num) && (*bIt == i_block_num))
		{
			if (i_append)
			{
				AF_ERR << "Try this task next: Already has a task " << *bIt << ":" << *tIt;
				return;
			}

			m_try_this_tasks_num.erase(tIt);
			m_try_this_blocks_num.erase(bIt);

			return;
		}

		tIt++;
		bIt++;
	}

	if (i_append)
	{
		m_try_this_tasks_num.push_back(i_task_num);
		m_try_this_blocks_num.push_back(i_block_num);
	}
	else
		AF_ERR << "Try this task next: Do not trying a task " << *bIt << ":" << *tIt;
}

RenderAf * JobAf::v_solve( std::list<RenderAf*> & i_renders_list, MonitorContainer * i_monitoring, BranchSrv * i_branch)
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

	// First we solving tasks if users asked to try them next
	if (hasTasksToTryNext() && checkTryTasksNext())
	{
		for (int i = 0; i < m_try_this_tasks_num.size(); i++)
		{
			int t = m_try_this_tasks_num[i];
			int b = m_try_this_blocks_num[i];

			if (false == (m_progress->tp[b][t]->state & AFJOB::STATE_READY_MASK))
				continue;

			if (false == (m_progress->tp[b][t]->state & AFJOB::STATE_TRYTHISTASKNEXT_MASK))
				m_progress->tp[b][t]->state |= AFJOB::STATE_TRYTHISTASKNEXT_MASK;

			if (solveTaskOnRender(i_render, b, t, i_monitoring))
				return true;
		}
	}

	// Next we just solving blocks in a cycle
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

		if (solveTaskOnRender(i_render, b, task_num, i_monitoring))
			return true;
	}

	return false;
}

bool JobAf::solveTaskOnRender(RenderAf * i_render, int i_block_num, int i_task_num, MonitorContainer * i_monitoring)
{
	af::TaskExec * task_exec = genTask(i_render, i_block_num, i_task_num);

	// No task was generated:
	if (NULL == task_exec)
	{
		return false;
	}

	// Check if render is online
	// It can be solved with offline render to check whether to WOL wake it
	if (i_render->isOffline())
	{
		delete task_exec;
		return true;
	}

	// Fill some TaskExec fields:
	task_exec->setJobName(m_name);
	task_exec->setUserName(m_user_name);
	if (hasCustomData())
		task_exec->setDataString("job_custom_data", m_custom_data);
	if (i_render->hasCustomData())
		task_exec->setDataString("render_custom_data", i_render->getCustomData());
	if (m_user->hasCustomData())
		task_exec->setDataString("user_custom_data", m_user->getCustomData());

	// Job was not able to start a task.
	// This should not happen.
	// This is some error situation (probably system job), see server log.
	if( false == m_blocks[task_exec->getBlockNum()]->v_startTask( task_exec, i_render, i_monitoring))
	{
		delete task_exec;
		return false;
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

af::TaskExec * JobAf::genTask(RenderAf * i_render, int i_block, int i_task)
{
	if (false == (m_blocks_data[i_block]->getState() & AFJOB::STATE_READY_MASK))
		return NULL;

	if (i_task >= m_blocks_data[i_block]->getTasksNum())
	{
		AF_ERR << "block[" << i_block << "] '" << m_blocks_data[i_block]->getName()
		       << "' : " << i_task << " >= number of tasks = " << m_blocks_data[i_block]->getTasksNum();
		return NULL;
	}

	if (false == (m_progress->tp[i_block][i_task]->state & AFJOB::STATE_READY_MASK))
		return NULL;

	if (false == m_blocks[i_block]->canRunOn(i_render))
		return NULL;

	if (m_blocks[i_block]->m_tasks[i_task]->avoidHostsCheck(i_render->getName()))
		return NULL;

	return m_blocks_data[i_block]->genTask(i_task);
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
	
	if (m_deletion)
	{
// Since running_tasks_num is calculated and stored in Af::Work, refresh no needed on deletion
//		for (int b = 0; b < m_blocks_num; b++)
//			m_blocks[b]->v_refresh(currentTime, renders, monitoring);

		if (getRunningTasksNum() == 0)
		{
			deleteNode(NULL, monitoring);
			// No later refresh needed, as running tasks number is stoted and calculated in af::Work
			return;
		}
	}

	// No more calculations needed for a locked job:
	if (isLocked())
		return;

	// Skip DONE job refresh if it is not forced:
	if (isDone() && (false == m_force_refresh))
		return;

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

	// Check block depends after all blocks refresh finished,
	// as states can be changed during refresh.
	// ( some block can be DONE after refresh )
	for (int b = 0; b < m_blocks_num; b++)
		if (m_blocks[b]->checkBlockDependStatus(monitoring))
			jobchanged = af::Monitor::EVT_jobs_change;

	// Set ready tasks to try next
	if (checkTryTasksNext())
		for (int i = 0; i < m_try_this_tasks_num.size(); i++)
		{
			int t = m_try_this_tasks_num[i];
			int b = m_try_this_blocks_num[i];
			if (m_progress->tp[b][t]->state & AFJOB::STATE_READY_MASK)
				m_progress->tp[b][t]->state |= AFJOB::STATE_TRYTHISTASKNEXT_MASK;
		}

	// Some statistics calculations:
	int32_t _tasks_error = 0;
	int32_t _tasks_ready = 0;
	for (int b = 0; b < m_blocks_num; b++)
	{
		_tasks_error += m_blocks[b]->m_data->getProgressTasksError();

		if (m_blocks[b]->m_data->getState() & AFJOB::STATE_READY_MASK)
			_tasks_ready += m_blocks[b]->m_data->getProgressTasksReady();
	}
	// Compare changes
	if ((_tasks_ready != m_tasks_ready) ||
		(_tasks_error != m_tasks_error))
		jobchanged = af::Monitor::EVT_jobs_change;
	// Store new calculations
	m_tasks_ready    = _tasks_ready;
	m_tasks_error    = _tasks_error;

	//
	// job state calculation
	m_state = m_state |   AFJOB::STATE_DONE_MASK;
	m_state = m_state & (~AFJOB::STATE_RUNNING_MASK);
	m_state = m_state & (~AFJOB::STATE_WARNING_MASK);
	m_state = m_state & (~AFJOB::STATE_SUSPENDED_MASK);
	m_state = m_state & (~AFJOB::STATE_ERROR_MASK);
	m_state = m_state & (~AFJOB::STATE_READY_MASK);
	m_state = m_state & (~AFJOB::STATE_SKIPPED_MASK);

	for( int b = 0; b < m_blocks_num; b++)
	{
		uint32_t state_block = m_blocks_data[b]->getState();
		m_state  = m_state | (state_block &   AFJOB::STATE_RUNNING_MASK   );
		m_state  = m_state | (state_block &   AFJOB::STATE_WARNING_MASK   );
		m_state  = m_state | (state_block &   AFJOB::STATE_SUSPENDED_MASK );
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
			if (hasCustomData() || m_user->hasCustomData())
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

	if (isDone())
		m_force_refresh = false;
}

void JobAf::emitEvents(const std::vector<std::string> & i_events) const
{
	// Processing command for system job if some events happened:
	if (i_events.empty())
		return;
	
	std::ostringstream str;
	str << "{";

	str << "\n\"user\":";
	m_user->v_jsonWrite(str, af::Msg::TUsersList);

	str << ",\n\"job\":";
	v_jsonWrite(str, af::Msg::TJobsList);

	str << ",\n\"events\":[";

	for (int i = 0; i < i_events.size(); i++)
	{
		if (i) str << ',';
		str << '"' << i_events[i] << '"';
	}
	str << "]\n}";
	
	SysJob::AddEventCommand(str.str(),
		"", // working directory - no matter
		m_user_name, m_name, i_events[0]);
}

void JobAf::restartAllTasks(const std::string & i_message, RenderContainer * i_renders, MonitorContainer * i_monitoring, uint32_t i_with_state)
{
	for( int b = 0; b < m_blocks_num; b++)
	{
		int numtasks = m_blocks_data[b]->getTasksNum();
		for( int t = 0; t < numtasks; t++)
		{
			m_blocks[b]->m_tasks[t]->operation(i_message, i_renders, i_monitoring, i_with_state, AFJOB::STATE_READY_MASK);
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
			mcblocks.addNode(m_blocks_data[i_block_ids[b]]);

		return new af::Msg(af::BlockData::DataModeFromString(i_modes[0]), &mcblocks);
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

	if (i_mode == "info")
	{
		af::TaskExec * exec = m_blocks[i_b]->m_tasks[i_t]->genExec();
		if (exec)
		{
			mctask.setExec(exec);
			return mctask.generateMessage(i_binary);
		}
		else
		{
			return af::jsonMsgError("af::TaskExec generation error on server.");
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

void JobAf::appendBlocks(Action & i_action, const JSON & i_operation)
{
	if (m_id == AFJOB::SYSJOB_ID)
	{
		i_action.answerError("Appending system job is not allowed.");
		return;
	}

	const JSON & blocks = i_operation["blocks"];
	if (!blocks.IsArray())
	{
		i_action.answerError("Operation requires blocks array.");
		return;
	}
	if (blocks.Size() == 0)
	{
		i_action.answerError("Operation blocks array has zero size.");
		return;
	}

	int old_blocks_num = m_blocks_num;

	if (false == jsonReadAndAppendBlocks(blocks))
	{
		i_action.answerError("Appending blocks failed, see server log for details.");
	}

	m_progress->reconstruct( this);

	// construct new blocks only (reuse the old_blocks_num existing ones)
	construct( old_blocks_num);

	// initialize and constuct new blocks ids in an answer
	std::string answer = "{\"block_ids\":[";
	for( int b = old_blocks_num; b < m_blocks_num; b++)
	{
		m_blocks_data[b]->setJobId( m_id);
		m_blocks[b]->storeTasks();
		m_blocks[b]->setUser( m_user);

		// Emit an event for monitors (afwatch ListTasks)
		i_action.monitors->addBlock(af::Msg::TBlocks, m_blocks[b]->m_data);

		if (b != old_blocks_num)
			answer += ",";
		answer += af::itos(b);
	}
	answer += "]}";
	i_action.answerObject(answer);

	checkDepends();
	checkStatesOnAppend();

	// Emit an event for monitors (afwatch ListJobs)
	i_action.monitors->addJobEvent(af::Monitor::EVT_jobs_change, getId(), getUid());
}
