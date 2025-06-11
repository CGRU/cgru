#pragma once

#include "../include/afjob.h"

#include "../libafanasy/msgclasses/mctaskup.h"
#include "../libafanasy/render.h"
#include "../libafanasy/renderevents.h"
#include "../libafanasy/renderupdate.h"
#include "../libafanasy/taskexec.h"

#include "afnodefarm.h"
#include "poolsrv.h"

class Action;
class JobContainer;
class MsgQueue;
class PoolsContainer;
class RenderContainer;

/// Afanasy server side of Render host.
class RenderAf : public af::Render, public AfNodeFarm
{
  public:
	/// Construct Render from message and provided address.
	RenderAf(af::Msg *msg);
	~RenderAf();

	/// Construct an offline render for store.
	RenderAf(const std::string &i_store_dir);

	/// Set registration time ( and update time).
	void setRegistered(PoolsContainer *i_pools);

	void setPool(PoolSrv *i_pool);

	void getPoolConfig();

	/// Awake offline render
	void online(RenderAf *render, JobContainer *i_jobs, MonitorContainer *monitoring);

	inline int findMaxTasks() const
	{
		if (m_max_tasks_host < 0 && m_parent)
			return m_parent->findMaxTasksHost();
		else
			return m_max_tasks_host;
	}
	inline int findCapacity() const
	{
		if (m_capacity_host < 0 && m_parent)
			return m_parent->findCapacityHost();
		else
			return m_capacity_host;
	}

	inline int findCapacityFree() const { return findCapacity() - m_capacity_used; }
	inline bool hasCapacity(int value) const
	{
		int c = findCapacity();
		if (c < 0)
			return true;
		else
			return m_capacity_used + value <= c;
	}

	inline int calcPoolPriority() const
	{
		if (m_parent)
			return m_parent->calcPriority();
		else
			return 0;
	}

	inline const std::string &findProperties() const
	{
		if (m_properties_host.empty() && m_parent)
			return m_parent->findPropertiesHost();
		else
			return m_properties_host;
	}
	inline int findPower() const
	{
		if (m_power_host < 0 && m_parent)
			return m_parent->findPowerHost();
		else
			return m_power_host;
	}

	/// Whether Render is ready to render tasks.
	inline bool isReady() const
	{
		return ((m_parent != NULL) && (isNotSick()) && (m_state & SOnline) && (m_priority > 0) &&
				((findCapacity() < 0) || (m_capacity_used < findCapacity())) &&
				((int)m_tasks.size() < findMaxTasks()) && (m_parent->isReady()) && (false == isWOLFalling()));
	}

	inline bool isWOLWakeAble() const
	{
		return ((m_parent != NULL) && (isNotSick()) && isOffline() && isWOLSleeping() &&
				(false == isWOLWaking()) && (findCapacity() != 0) && (findMaxTasks() > 0) &&
				(m_parent->isReady()) && (m_priority > 0));
	}

	bool hasTickets(const std::map<std::string, int32_t> &i_tickets) const;

	/// Add task \c taskexec to render, \c start or only capture it
	/// Takes over the taskexec ownership
	void setTask(af::TaskExec *taskexec, MonitorContainer *monitoring, bool start = true);

	/// Start tast \c taskexec on remote render host, task must be set before and exists on render.
	void startTask(af::TaskExec *taskexec);

	/// Make Render to stop task.
	void stopTask(int jobid, int blocknum, int tasknum, int number);

	/// Make Render to stop task.
	inline void stopTask(const af::TaskExec *taskexec)
	{
		stopTask(taskexec->getJobId(), taskexec->getBlockNum(), taskexec->getTaskNum(),
				 taskexec->getNumber());
	}

	/// Make Render to stop task.
	inline void stopTask(const af::MCTaskUp &taskup)
	{
		stopTask(taskup.getNumJob(), taskup.getNumBlock(), taskup.getNumTask(), taskup.getNumber());
	}

	/// Make Render to finish task.
	/// Releases the ownership of taskexec (Render will not own it any more)
	void taskFinished(const af::TaskExec *i_exec, int64_t i_state, MonitorContainer *i_monitoring);

	/// Refresh parameters.
	void v_refresh(time_t i_current_time, AfContainer *pointer, MonitorContainer *monitoring) override;

	// Perform post solving calculations:
	void v_postSolve(time_t i_current_time, MonitorContainer *i_monitoring);

	af::Msg *writeTasksLog(bool i_binary);

	/// Deregister render, on SIGINT client recieving.
	void deregister(JobContainer *jobs, MonitorContainer *monitoring);

	virtual void v_action(Action &i_action) override;

	inline const std::list<std::string> &getTasksLog() { return m_tasks_log; } ///< Get tasks log list.

	virtual int v_calcWeight() const; ///< Calculate and return memory size.

	// Write this message back to the new launched render
	af::Msg *writeConnectedMsg(const std::string &i_log);

	// Update render and send instructions back:
	af::Msg *update(const af::RenderUpdate &i_up);

	// Need for server to write some farm parameters (gui double-click):
	af::Msg *writeFullInfo(bool binary) const;

	void wolWake(MonitorContainer *i_monitoring, const std::string &i_msg = std::string());

	inline void addTaskOutput(const af::MCTaskPos &i_tp) { m_re.addTaskOutput(i_tp); }

	inline void listenTask(const af::MCTaskPos &i_tp, bool i_subscribe)
	{
		if (i_subscribe)
			m_re.taskListenAdd(i_tp);
		else
			m_re.taskListenRem(i_tp);
	}

	void actionHealSick(Action &i_action);

	void launchAndExit(const std::string &i_cmd, bool i_exit);

	void ejectTasks(JobContainer *jobs, MonitorContainer *monitoring, uint32_t upstatus,
					const std::string *i_keeptasks_username = NULL);

	void exitClient(const std::string &i_type, JobContainer *i_jobs, MonitorContainer *i_monitoring);

	/// Set Render to Ofline. \c updateTaskState - whether to update it's state.
	void offline(JobContainer *jobs, uint32_t updateTaskState, MonitorContainer *monitoring,
				 bool toZombie = false);

  public:
	/// Set container:
	inline static void setRenderContainer(RenderContainer *i_container) { ms_renders = i_container; }

	/// Stop task on render if it is not running (for server)
	static void closeLostTask(const af::MCTaskUp &taskup);

  private:
	void initDefaultValues();

	void findPool(PoolsContainer *i_pools);

	bool actionSetPool(const std::string &i_pool_name, Action &i_action);
	bool actionReassignPool(Action &i_action);

	// Write this message back to the new launched render
	af::Msg *writeRenderEventsMsg();

	/// Add the task exec to this render and take over its ownership (meaning
	/// one should not free taskexec after having provided it to this method).
	void addTask(af::TaskExec *i_taskexec, MonitorContainer *i_monitoring);
	/// Remove the task exec from this render and give back its ownership to the
	/// caller.
	void removeTask(const af::TaskExec *i_taskexec, MonitorContainer *i_monitoring);

	void addErrorTask(const af::TaskExec *i_exec);
	void clearErrorTasks();

	void wolSleep(MonitorContainer *monitoring);

	void appendTasksLog(const std::string &message); ///< Append tasks log with a \c message .

	/**
	 * @brief Emit events and submit them to the system job
	 * @param events: event types, e.g. RENDER_ZOMBIE or RENDER_SICK
	 */
	void emitEvents(const std::vector<std::string> &i_events) const;

	// Check resources, on overflow emit RENDER_OVERLOAD event
	bool checkOverload();

  private:
	std::list<std::string> m_tasks_log; ///< Tasks Log.

	af::RenderEvents m_re;

	struct ErrorTaskData
	{
		int64_t when;
		std::string service;
		std::string user_name;
	};

	std::list<ErrorTaskData *> m_error_tasks;

	int64_t m_no_task_time;
	int m_no_task_event_count;

	int64_t m_overload_time;
	int m_overload_seconds;

  private:
	static RenderContainer *ms_renders;
};
