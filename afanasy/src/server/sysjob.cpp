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
#include "sysjob_events.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   COMMAND    ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

SysCmd::SysCmd(
		const std::string & i_command,
		const std::string & i_working_directory,
		const std::string & i_user_name,
		const std::string & i_job_name,
		const std::string & i_task_name
	):
	command( i_command),
	working_directory( i_working_directory),
	user_name( i_user_name),
	job_name( i_job_name),
	task_name( i_task_name)
{
	if( task_name.empty())
	{
		task_name = command.substr( 0, AFJOB::SYSJOB_TASKSNAMEMAX);
		size_t space = task_name.find(' ', 1);
		if( space != std::string::npos )
			task_name = task_name.substr( 0, space);
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   TASK    //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

SysTask::SysTask( af::TaskExec * i_taskexec, SysCmd * i_system_command, Block * i_block, int i_task_number):
	Task( i_block, &m_taskProgress, i_task_number),
	m_syscmd( i_system_command),
	m_birthtime(0)
{
AFINFO("SysTask::SysTask:");
	m_progress = &m_taskProgress;
}

SysTask::~SysTask()
{
// Delete system command data:
	delete m_syscmd;
}

std::string const SysTask::v_getInfo(bool full) const
{
	std::string info = "#";
	info += af::itos( getNumber()) + ": ";
	info += af::state2str( m_progress->state) + ": ";
	info += m_syscmd->command;
	return info;
}

void SysTask::v_monitor( MonitorContainer * monitoring) const {}
void SysTask::v_updateDatabase() const {}
void SysTask::v_writeTaskOutput( const af::MCTaskUp& taskup) const {}

void SysTask::v_appendLog( const std::string & message)
{
	((SysBlock*)(m_block))->appendTaskLog( std::string("#") + af::itos( getNumber()) + ": " + message + ": "
       + m_syscmd->user_name + ": \"" + m_syscmd->job_name + "\":\n"
       + m_syscmd->command);
}

void SysTask::appendSysJobLog( const std::string & message)
{
	SysJob::appendJobLog( std::string("Task[") + af::itos( getNumber()) + "]: " + message + ": "
	    + m_syscmd->user_name + ": \"" + m_syscmd->job_name + "\":\n"
	    + m_syscmd->command);
}

void SysTask::v_start( af::TaskExec * taskexec, int * runningtaskscounter, RenderAf * render, MonitorContainer * monitoring)
{
	taskexec->setName(         m_syscmd->task_name        );
	taskexec->setCommand(      m_syscmd->command          );
	taskexec->setUserName(     m_syscmd->user_name        );
	taskexec->setJobName(      m_syscmd->job_name         );
	taskexec->setWDir(         m_syscmd->working_directory);
	taskexec->setTaskNumber(   getNumber()                );

	Task::v_start( taskexec, runningtaskscounter, render, monitoring);
}

void SysTask::v_refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring, int & errorHostId)
{
AFINFO("SysTask::refresh:");
	Task::v_refresh( currentTime, renders, monitoring, errorHostId);
	if( m_birthtime == 0 ) m_birthtime = currentTime;
	if((currentTime - m_birthtime > af::Environment::getSysJobTaskLife() ) && (isReady()))
	{
		std::string message = std::string("Error: Task age(") + af::itos( currentTime - m_birthtime) + ") > " + af::itos( af::Environment::getSysJobTaskLife());
		v_appendLog( message);
		// Store error in job log
		appendSysJobLog( message);
		m_progress->state = AFJOB::STATE_ERROR_MASK;
	}
//stdOut();
}

void SysTask::v_updateState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring, bool & errorHost)
{
//printf("SysTask::updateState: "); taskup.stdOut( true);
	Task::v_updateState( taskup, renders, monitoring, errorHost);
//stdOut();

	// Store error messages and logs:

	std::string message;
	switch ( taskup.getStatus())
	{
		case af::TaskExec::UPWarning:                   message = "Warning";             break;
		case af::TaskExec::UPFailedToStart:             message = "Failed to start";     break;
		case af::TaskExec::UPFinishedKilled:            message = "Finished crashed";    break;
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
		((SysBlock*)(m_block))->appendTaskLog(message);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   BLOCK    /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Task * SysBlock::task = NULL;

SysBlock::SysBlock( JobAf * blockJob, af::BlockData * blockData, af::JobProgress * progress):
	Block( blockJob, blockData, progress)
{
AFINFO("SysBlock::SysBlock:");
	m_taskprogress = progress->tp[ m_data->getBlockNum()][0];
	m_taskprogress->state &= ~AFJOB::STATE_READY_MASK;
	m_taskprogress->starts_count = 0;
	m_taskprogress->errors_count = 0;
}

SysBlock::~SysBlock()
{
	for( std::list<SysCmd *>::iterator it = m_commands.begin(); it != m_commands.end(); it++) delete *it;
	for( std::list<SysTask*>::iterator it = m_systasks.begin(); it != m_systasks.end(); it++) delete *it;
}

void SysBlock::addCommand( SysCmd * syscmd)
{
	m_commands.push_back( syscmd);
	m_taskprogress->state |= AFJOB::STATE_READY_MASK;
}

bool SysBlock::isReady() const
{
	bool ready = true;

	if( getReadySysTask() == NULL )
	{
		// Block is ready only if commands exists and tasks number below some limit:
		if(( getNumCommands() < 1 ) || ( getNumSysTasks() >= af::Environment::getSysJobTasksMax() ))
		{
			m_taskprogress->state &= ~AFJOB::STATE_READY_MASK;
			ready = false;
		}
	}
	if( ready ) m_taskprogress->state |= AFJOB::STATE_READY_MASK;

	return ready;
}

void SysBlock::v_startTask( af::TaskExec * taskexec, RenderAf * render, MonitorContainer * monitoring)
{
AFINFO("SysBlock::startTask:");
	taskexec->setBlockName( m_data->getName());
	SysTask * systask = getReadySysTask();

	// Add new ready task:
	if( systask == NULL ) systask = addTask( taskexec);

	if( systask == NULL ) return;

	systask->v_start( taskexec, m_data->getRunningTasksCounter(), render, monitoring);

	m_taskprogress->state |= AFJOB::STATE_RUNNING_MASK;
	m_taskprogress->starts_count++;

	if( monitoring ) m_tasks[0]->v_monitor( monitoring);
}

SysTask * SysBlock::getReadySysTask() const
{
	for( std::list<SysTask*>::const_iterator it = m_systasks.begin(); it != m_systasks.end(); it++)
		if( (*it)->isReady() ) return *it;
	return NULL;
}

SysTask * SysBlock::addTask( af::TaskExec * taskexec)
{
AFINFO("SysBlock::addTask:");
	if( m_commands.size() == 0)
	{
		AFCommon::QueueLogError("SysBlock::addTask: commands.size() == 0");
		return NULL;
	}

	// Get the first command:
	SysCmd * command = m_commands.front();

	// Get the smallest task number:
	int number = 0;
	bool founded;
	for( ; number < af::Environment::getSysJobTasksMax(); number++)
	{
		founded = true;
		for( std::list<SysTask*>::iterator it = m_systasks.begin(); it != m_systasks.end(); it++)
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
	m_commands.pop_front();

	// Add system task:
	m_systasks.push_back( systask);

	return systask;
}

void SysBlock::clearCommands()
{
//printf("SysBlock::clearCommands:\n");
	for( std::list<SysCmd *>::iterator it = m_commands.begin(); it != m_commands.end(); it++) delete *it;
	m_commands.clear();
	m_taskprogress->starts_count = 0;
	m_taskprogress->errors_count = 0;
}

void SysBlock::v_errorHostsAppend( int task, int hostId, RenderContainer * renders)
{
	RenderContainerIt rendersIt( renders);
	RenderAf* render = rendersIt.getRender( hostId);
	if( render == NULL ) return;
	if( Block::v_errorHostsAppend( render->getName())) appendJobLog( render->getName() + " - AVOIDING HOST !");
	SysTask * systask = getTask( task, "errorHostsAppend");
	if( systask) systask->errorHostsAppend( render->getName());
}

void SysBlock::v_getErrorHostsList( std::list<std::string> & o_list) const
{
	Block::v_getErrorHostsList( o_list);
	for( std::list<SysTask*>::const_iterator it = m_systasks.begin(); it != m_systasks.end(); it++)
		(*it)->getErrorHostsList( o_list);
}

void SysBlock::updateTaskState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("SysBlock::updateTaskState:\n");
	SysTask * systask = getTask( taskup.getNumTask(), "updateTaskState");
	if( systask == NULL ) return;
	bool errorHost = false;
	systask->v_updateState( taskup, renders, monitoring, errorHost);
	if( errorHost) v_errorHostsAppend( taskup.getNumTask(), taskup.getClientId(), renders);
}

bool SysBlock::v_refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring)
{
AFINFO("SysBlock::refresh:");
	bool blockProgress_changed = false;
	bool taskchanged = false;

	uint32_t blockstate_new = m_data->getState();
	uint32_t blockstate_old = blockstate_new;
	uint32_t taskstate_old = m_taskprogress->state;
	int tasksready_old = m_data->getProgressTasksReady();
	int tasksdone_old  = m_data->getProgressTasksDone();
	int taskserror_old = m_data->getProgressTasksError();
	int tasksready_new = 0;
	int tasksdone_new  = tasksdone_old;
	int taskserror_new = 0;

	m_taskprogress->state &= ~AFJOB::STATE_RUNNING_MASK;
	m_taskprogress->state &= ~AFJOB::STATE_READY_MASK;

	for( std::list<SysTask*>::iterator it = m_systasks.begin(); it != m_systasks.end(); it++)
	{
		int errorHostId = -1;
		(*it)->v_refresh( currentTime, renders, monitoring, errorHostId);
		if( errorHostId != -1 ) v_errorHostsAppend( 0, errorHostId, renders);
		if((*it)->isRunning()) m_taskprogress->state |= AFJOB::STATE_RUNNING_MASK;
		if((*it)->isReady()  )
		{
			m_taskprogress->state |= AFJOB::STATE_READY_MASK;
			taskserror_new ++;
		}
	}

	tasksdone_new += deleteFinishedTasks( taskchanged);

	if( m_commands.size())
	{
		m_taskprogress->state |= AFJOB::STATE_READY_MASK;
		tasksready_new = m_commands.size();
	}


	// Set block state accoring to task state
	if( m_taskprogress->state & AFJOB::STATE_READY_MASK )
	   blockstate_new |= AFJOB::STATE_READY_MASK;
	else
	   blockstate_new &= ~AFJOB::STATE_READY_MASK;

	if( m_taskprogress->state & AFJOB::STATE_RUNNING_MASK )
	   blockstate_new |= AFJOB::STATE_RUNNING_MASK;
	else
	   blockstate_new &= ~AFJOB::STATE_RUNNING_MASK;

	m_data->setState( blockstate_new);

	// Check changes:
	if(  taskstate_old  != m_taskprogress->state ) taskchanged = true;
	if(( blockstate_old != blockstate_new ) ||
		( tasksready_old != tasksready_new ) ||
		( tasksdone_old  != tasksdone_new  ) ||
		( taskserror_old != taskserror_new )  ) blockProgress_changed = true;

	if( taskchanged && monitoring) m_tasks[0]->v_monitor( monitoring);


	// For block in jobs list monitoring
	if( Block::v_refresh( currentTime, renders, monitoring))
	{
		// If block progress changed there, the function will add block in monitoring itself
		blockProgress_changed = true;
	}
	else
	{
		// Add block to monitoring if it was not, but has changes
		if( monitoring ) monitoring->addBlock( af::Msg::TBlocksProgress, m_data);
	}

	m_data->setProgressTasksReady( tasksready_new  );
	m_data->setProgressTasksDone(  tasksdone_new   );
	m_data->setProgressTasksError( taskserror_new  );

	return blockProgress_changed;
}

int SysBlock::deleteFinishedTasks( bool & taskProgressChanged)
{
	int done_tasks = 0;
	taskProgressChanged = false;
	for( std::list<SysTask*>::iterator it = m_systasks.begin(); it != m_systasks.end();)
	{
		if(( false == (*it)->isRunning()) && ((*it)->isError() || (*it)->isDone()))
		{
			if((*it)->isError())
			{
				m_taskprogress->errors_count++;
				taskProgressChanged = true;
			}
			else done_tasks++;
			delete *it;
			it = m_systasks.erase( it);
		}
		else it++;
	}
	return done_tasks;
}

SysTask * SysBlock::getTask( int tasknum, const char * errorMessage)
{
	for( std::list<SysTask*>::iterator it = m_systasks.begin(); it != m_systasks.end(); it++)
		if( tasknum == (*it)->getNumber()) return *it;
	if( errorMessage ) AFCommon::QueueLogError( std::string("SysJob::getTask: ") + errorMessage + ": Invalid task number = " + af::itos(tasknum));
	else               AFCommon::QueueLogError( std::string("SysJob::getTask: Invalid task number = ") + af::itos(tasknum));
	return NULL;
}

void SysBlock::v_errorHostsReset()
{
//printf("SysBlock::errorHostsReset:\n");
	Block::v_errorHostsReset();
	for( std::list<SysTask*>::iterator it = m_systasks.begin(); it != m_systasks.end(); it++) (*it)->errorHostsReset();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   JOB    ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

SysJob   * SysJob::ms_sysjob        = NULL;
SysBlock * SysJob::ms_block_cmdpost = NULL;
SysBlock * SysJob::ms_block_wol     = NULL;
SysBlock * SysJob::ms_block_events  = NULL;

SysJob::SysJob( int flags):
	JobAf( AFJOB::SYSJOB_ID)
{
	ms_sysjob = this;

	if( flags != New ) return;

	m_name              = AFJOB::SYSJOB_NAME;
	m_user_name         = AFJOB::SYSJOB_USERNAME;
	m_priority          = AFGENERAL::DEFAULT_PRIORITY;
	m_max_running_tasks = AFGENERAL::MAXRUNNINGTASKS;

	m_blocksnum = BlockLastIndex;
	m_blocksdata = new af::BlockData*[m_blocksnum];
	m_blocksdata[BlockPostCmdIndex] = new SysBlockData_CmdPost( BlockPostCmdIndex, m_id);
	m_blocksdata[BlockWOLIndex    ] = new SysBlockData_WOL(     BlockWOLIndex,     m_id);
	m_blocksdata[BlockEventsIndex ] = new SysBlockData_Events(  BlockEventsIndex,  m_id);

	progress = new afsql::DBJobProgress( this);

	construct();
	m_fromdatabase = false;

	printf("System job constructed.\n");
}

SysJob::~SysJob()
{
}

void SysJob::v_dbDelete( std::list<std::string> * queries) const
{
	AFCommon::QueueLogError("Trying to delete system job from database.");
}

void SysJob::v_setZombie( RenderContainer * renders, MonitorContainer * monitoring)
{
	AFCommon::QueueLogError("Trying to make system job a zomibe.");
}

Block * SysJob::v_newBlock( int numBlock)
{
	switch( numBlock)
	{
	case BlockPostCmdIndex:
	{
		ms_block_cmdpost = new SysBlock_CmdPost( this, m_blocksdata[numBlock], progress);
		return ms_block_cmdpost;
	}
	case BlockWOLIndex:
	{
		ms_block_wol = new SysBlock_WOL( this, m_blocksdata[numBlock], progress);
		return ms_block_wol;
	}
	case BlockEventsIndex:
	{
		ms_block_events = new SysBlock_Events( this, m_blocksdata[numBlock], progress);
		return ms_block_events;
	}
	default:
		AFERRAR("SysJob::createBlock: Invalid block number = %d", numBlock)
	}
	return NULL;
}

void SysJob::AddPostCommand( const std::string & i_cmd, const std::string & i_wdir, const std::string & i_user_name, const std::string & i_job_name)
{
	ms_block_cmdpost->addCommand( new SysCmd( i_cmd, i_wdir, i_user_name, i_job_name));
}
void SysJob::AddWOLCommand( const std::string & i_cmd, const std::string & i_wdir, const std::string & i_user_name, const std::string & i_job_name)
{
	ms_block_wol->addCommand( new SysCmd( i_cmd, i_wdir, i_user_name, i_job_name));
}
void SysJob::AddEventCommand( const std::string & i_cmd, const std::string & i_wdir, const std::string & i_user_name, const std::string & i_job_name, const std::string & i_task_name)
{
//printf("SysJob::AddEventCommand:\n%s\n", i_cmd.c_str());
	ms_block_events->addCommand( new SysCmd( i_cmd, i_wdir, i_user_name, i_job_name, i_task_name));
}

bool SysJob::v_solve( RenderAf *render, MonitorContainer * monitoring)
{
//printf("SysJob::solve:\n");
	for( int b = 0; b < m_blocksnum; b++ )
		if(((SysBlock*)(m_blocks[b]))->isReady())
			return JobAf::v_solve( render, monitoring);

	return false;
}

void SysJob::v_updateTaskState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("SysJob::updateTaskState:\n");
//   JobAf::updateTaskState( taskup, renders, monitoring)

	if( taskup.getNumBlock() >= BlockLastIndex )
	{
		AFCommon::QueueLogError("SysJob::updateTaskState: Invalid block number = " + af::itos(taskup.getNumBlock()));
		return;
	}

	((SysBlock*)(m_blocks[taskup.getNumBlock()]))->updateTaskState( taskup, renders, monitoring);
}

void SysJob::v_refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
AFINFO("SysJob::refresh:");
	JobAf::v_refresh( currentTime, pointer, monitoring);
}

void SysJob::v_restartTasks( const af::MCTasksPos &taskspos, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("SysJob::restartTasks:\n");
	for( int p = 0; p < taskspos.getCount(); p++)
	{
		int b = taskspos.getNumBlock(p);
		if( b >= m_blocksnum)
		{
			AFERRAR("SysJob::skipTasks: b >= blocksnum ( %d >= %d )", b, m_blocksnum)
			continue;
		}
		((SysBlock*)(m_blocks[b]))->clearCommands();
	}
	JobAf::v_restartTasks( taskspos, renders, monitoring);
}

const std::string SysJob::v_getErrorHostsListString( int b, int t) const
{
	return "This is an empty dummy task in s system job block.\nGet job error hosts list to see its tasks error hosts.";
}

af::Msg * SysJob::v_getTaskStdOut( int i_b, int i_t, int i_n, RenderContainer * i_renders,
	std::string & o_filename, std::string & o_error) const
{
	o_error = "This is an empty dummy task in a system job block.\nError tasks output are stored in this task log.";
	return NULL;
}

void SysJob::appendJobLog( const std::string & message)
{
	ms_sysjob->appendLog( message);
}

bool SysJob::initSystem()
{
	if( m_blocksnum != BlockLastIndex ) return false;
	m_time_creation = time(NULL);
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   BLOCK DATA   //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SysBlockData::SysBlockData( int BlockNum, int JobId):
	afsql::DBBlockData( BlockNum, JobId)
{
//   initDefaults();
AFINFA("DBBlockData::DBBlockData: JobId=%d, BlockNum=%d", m_job_id, m_block_num)

	m_capacity = af::Environment::getTaskDefaultCapacity();

	m_name = "system_commands";

	m_tasks_num = 1;

	m_tasks_data = new af::TaskData*[m_tasks_num];
	for( int t = 0; t < m_tasks_num; t++)
	{
		m_tasks_data[t] = new SysTaskData;
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
	m_name = "Dummy task. See all tasks logs here.";
}

SysTaskData::~SysTaskData()
{
}
