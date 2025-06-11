#pragma once

#include "../libafanasy/blockdata.h"
#include "../libafanasy/taskdata.h"
#include "../libafanasy/taskprogress.h"

#include "block.h"
#include "jobaf.h"
#include "task.h"

class SysJob;

/// System job command:
struct SysCmd
{
	SysCmd(const std::string &i_command, const std::string &i_working_directory,
		   const std::string &i_user_name, const std::string &i_job_name,
		   const std::string &i_task_name = std::string());

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
	SysTask(af::TaskExec *i_taskexec, SysCmd *i_system_command, Block *i_block, int i_task_number);
	virtual ~SysTask();

	virtual void v_start(af::TaskExec *i_taskexec, RenderAf *i_render,
						 MonitorContainer *i_monitoring) override;
	virtual void v_refresh(time_t i_currentTime, RenderContainer *i_renders, MonitorContainer *i_monitoring,
						   int &i_errorHostId) override;
	virtual void v_updateState(const af::MCTaskUp &i_taskup, RenderContainer *i_renders,
							   MonitorContainer *i_monitoring, bool &i_errorHost) override;
	virtual void v_writeTaskOutput(const char *i_data,
								   int i_size) const override; ///< Write task output in tasksOutputDir.
	virtual const std::string v_getInfo(bool i_full = false) const override;
	virtual void v_appendLog(const af::Log &i_log, bool i_store = false) override;
	virtual void v_monitor(MonitorContainer *i_monitoring) const override;

	// This function does nothing in system job
	virtual void v_store() override;

  private:
	void appendSysJobLog(const std::string &i_message);

  private:
	SysCmd *m_syscmd;
	af::TaskProgress m_taskProgress;
	long long m_birthtime;
};

/// System job block:
class SysBlock : public Block
{
  public:
	SysBlock(JobAf *blockJob, af::BlockData *blockData, af::JobProgress *progress);
	virtual ~SysBlock();

	void updateTaskState(const af::MCTaskUp &taskup, RenderContainer *renders, MonitorContainer *monitoring);

	void addCommand(SysCmd *syscmd);
	bool isReady() const;

	void clearCommands();

	virtual bool v_refresh(time_t currentTime, RenderContainer *renders,
						   MonitorContainer *monitoring) override;
	virtual void v_getErrorHostsList(std::list<std::string> &o_list) const override;
	virtual bool v_startTask(af::TaskExec *taskexec, RenderAf *render, MonitorContainer *monitoring) override;
	virtual void v_errorHostsAppend(int task, int hostId, RenderContainer *renders) override;
	virtual void v_errorHostsReset() override;

	inline int getNumCommands() const { return m_commands.size(); }
	inline int getNumSysTasks() const { return m_systasks.size(); }
	SysTask *getReadySysTask() const;

	inline void appendTaskLog(af::Log &i_log, bool i_store = false) const
	{
		m_tasks[0]->v_appendLog(i_log, i_store);
	}

	bool initSystem();

  private:
	SysTask *addTask(af::TaskExec *taskexec);
	SysTask *getTask(int tasknum, const char *errorMessage = NULL);
	int deleteFinishedTasks(bool &taskProgressChanged);

  private:
	af::TaskProgress *m_taskprogress;

  private:
	std::list<SysCmd *> m_commands;
	std::list<SysTask *> m_systasks;
};

/// System job:
class SysJob : public JobAf
{
  public:
	static void AddPostCommand(const std::string &i_cmd, const std::string &i_wdir,
							   const std::string &i_user_name, const std::string &i_job_name);
	static void AddWOLCommand(const std::string &i_cmd, const std::string &i_wdir,
							  const std::string &i_user_name, const std::string &i_job_name);
	static void AddEventCommand(const std::string &i_cmd, const std::string &i_wdir,
								const std::string &i_user_name, const std::string &i_job_name,
								const std::string &i_task_name);

	inline static const JobAf *GetJob() { return ms_sysjob; }
	inline static const UserAf *GetUser() { return ms_sysjob->m_user; }

	enum BlocksEnum
	{
		BlockPostCmdIndex,
		BlockWOLIndex,
		BlockEventsIndex,
		BlockLastIndex
	};

  public:
	SysJob(const std::string &i_folder = "");
	virtual ~SysJob();

	bool initSystem();

	virtual bool v_canRun() override;
	virtual RenderAf *v_solve(std::list<RenderAf *> &i_renders_list, MonitorContainer *monitoring,
							  BranchSrv *i_branch) override;
	virtual void v_updateTaskState(const af::MCTaskUp &taskup, RenderContainer *renders,
								   MonitorContainer *monitoring) override;
	virtual void v_refresh(time_t currentTime, AfContainer *pointer, MonitorContainer *monitoring) override;

	static void appendSysJobLog(const std::string &message);

	// Functions than informate that it is a system task, and this info is not abailable:
	virtual void v_getTaskOutput(af::MCTask &io_mctask, std::string &o_error) const override;

  protected:
	virtual Block *v_newBlock(int numBlock) override;

  private:
	bool isReady();

	static SysJob *ms_sysjob;
	static SysBlock *ms_block_cmdpost;
	static SysBlock *ms_block_wol;
	static SysBlock *ms_block_events;
};

/// System job block data:
class SysBlockData : public af::BlockData
{
  public:
	SysBlockData(int BlockNum, int JobId);
	virtual ~SysBlockData();

	bool initSystem();
};

/// System job task data:
class SysTaskData : public af::TaskData
{
  public:
	SysTaskData();
	~SysTaskData();
};
