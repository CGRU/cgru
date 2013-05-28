#pragma once

#include "../libafanasy/taskprogress.h"

#include "../libafsql/dbblockdata.h"
#include "../libafsql/dbtaskdata.h"

#include "block.h"
#include "jobaf.h"
#include "task.h"

class SysJob;

/// System job command:
struct SysCmd
{
	SysCmd(
		const std::string & i_command,
		const std::string & i_working_directory,
		const std::string & i_user_name,
		const std::string & i_job_name,
		const std::string & i_task_name = std::string()
	);

	std::string command;
	std::string working_directory;
	std::string user_name;
	std::string job_name;
	std::string task_name;
};

/// System job task:
class SysTask : public Task
{
public:
	SysTask( af::TaskExec * i_taskexec, SysCmd * i_system_command, Block * i_block, int i_task_number);
	virtual ~SysTask();

	virtual void v_start( af::TaskExec * i_taskexec, int * i_runningtaskscounter, RenderAf * i_render, MonitorContainer * i_monitoring);
	virtual void v_refresh( time_t i_currentTime, RenderContainer * i_renders, MonitorContainer * i_monitoring, int & i_errorHostId);
	virtual void v_updateState( const af::MCTaskUp & i_taskup, RenderContainer * i_renders, MonitorContainer * i_monitoring, bool & i_errorHost);
	virtual void v_writeTaskOutput( const af::MCTaskUp & i_taskup) const;  ///< Write task output in tasksOutputDir.
	virtual const std::string v_getInfo( bool i_full = false) const;
	virtual void v_appendLog( const std::string & i_message);
	virtual void v_monitor( MonitorContainer * i_monitoring) const;
	virtual void v_updateDatabase() const;

private:
	void appendSysJobLog( const std::string & i_message);

private:
	SysCmd * m_syscmd;
	af::TaskProgress m_taskProgress;
	long long m_birthtime;
};

/// System job block:
class SysBlock : public Block
{
public:
	SysBlock( JobAf * blockJob, af::BlockData * blockData, af::JobProgress * progress);
	virtual ~SysBlock();

	void updateTaskState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring);

	void addCommand( SysCmd* syscmd);
	bool isReady() const;

	void clearCommands();

	virtual bool v_refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring);
	virtual void v_getErrorHostsList( std::list<std::string> & o_list) const;
	virtual void v_startTask( af::TaskExec * taskexec, RenderAf * render, MonitorContainer * monitoring);
	virtual void v_errorHostsAppend( int task, int hostId, RenderContainer * renders);
	virtual void v_errorHostsReset();

	inline int getNumCommands() const { return m_commands.size();}
	inline int getNumSysTasks() const { return m_systasks.size();}
	SysTask * getReadySysTask() const;

	inline void appendTaskLog( const std::string & message) const { m_tasks[0]->v_appendLog( message);}

private:
	SysTask * addTask( af::TaskExec * taskexec);
	SysTask * getTask( int tasknum, const char * errorMessage = NULL);
	int deleteFinishedTasks( bool & taskProgressChanged);

private:
	af::TaskProgress * m_taskprogress;

private:
	std::list<SysCmd*> m_commands;
	std::list<SysTask*> m_systasks;
};

/// System job:
class SysJob : public JobAf
{
public:
	static void AddPostCommand( const std::string & i_cmd, const std::string & i_wdir, const std::string & i_user_name, const std::string & i_job_name);
	static void AddWOLCommand( const std::string & i_cmd, const std::string & i_wdir, const std::string & i_user_name, const std::string & i_job_name);
	static void AddEventCommand( const std::string & i_cmd, const std::string & i_wdir, const std::string & i_user_name, const std::string & i_job_name, const std::string & i_task_name);

	enum CreationFlags
	{
		New,
		FromDataBase
	};

	enum BlocksEnum
	{
		BlockPostCmdIndex,
		BlockWOLIndex,
		BlockEventsIndex,
		BlockLastIndex
	};

public:
	SysJob( int m_flags);
	virtual ~SysJob();

	bool initSystem();

	virtual bool v_solve( RenderAf *render, MonitorContainer * monitoring);
	virtual void v_updateTaskState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring);
	virtual void v_refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);
	virtual void v_setZombie( RenderContainer * renders, MonitorContainer * monitoring);
	virtual void v_dbDelete( std::list<std::string> * queries) const;
	virtual void v_restartTasks( const af::MCTasksPos & taskspos, RenderContainer * renders, MonitorContainer * monitoring);  ///< Restart some tasks.

	static void appendJobLog( const std::string & message);

	// Functions than informate that it is a system task, and this info is not abailable:
	virtual af::Msg * v_getTaskStdOut( int i_b, int i_t, int i_n, RenderContainer * i_renders,
		std::string & o_filename, std::string & o_error) const;
	virtual const std::string v_getErrorHostsListString( int b, int t) const;

protected:
	virtual Block * v_newBlock( int numBlock);

private:
	static SysJob * ms_sysjob;
	static SysBlock * ms_block_cmdpost;
	static SysBlock * ms_block_wol;
	static SysBlock * ms_block_events;
};

/// System job block data:
class SysBlockData : public afsql::DBBlockData
{
public:
	SysBlockData ( int BlockNum, int JobId);
	virtual ~SysBlockData ();
};

/// System job task data:
class SysTaskData : public afsql::DBTaskData
{
public:
	SysTaskData();
	~SysTaskData();
};
