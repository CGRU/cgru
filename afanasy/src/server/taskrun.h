#pragma once

#include "../libafanasy/name_af.h"
#include "../libafanasy/taskexec.h"

class Block;
class RenderAf;
class RenderContainer;
class MonitorContainer;
class MsgAf;
class Task;

/// Afanasy running task class.
/** When task starts it handle all messages and render host.
*** Will be deleted on running session finish.
**/
class TaskRun
{
  public:
	/// Constructor.
	/** TaskRun run task in constructor if it is not multihost.
	*** Multihost tasks running session is handled by TaskRunMulti class, which inherits this class.
	**/
	TaskRun(Task *runningTask, af::TaskExec *taskExec, af::TaskProgress *taskProgress, Block *taskBlock,
			RenderAf *render, MonitorContainer *monitoring);

	virtual ~TaskRun();

	/// When running session finishes this class became a zombie - ready to delete.
	inline bool isZombie() const { return m_zombie; }
	inline bool notZombie() const { return m_zombie == false; }

	/// Update task state by sent message, almost often from remote render host
	virtual void update(const af::MCTaskUp &taskup, RenderContainer *renders, MonitorContainer *monitoring,
						bool &o_error_host);

	/// Do some work every period of time. Return true if there are some changes for database and monitoring.
	virtual bool refresh(time_t currentTime, RenderContainer *renders, MonitorContainer *monitoring,
						 int &errorHostId);

	/// Return running render id:
	virtual int v_getRunningRenderID(std::string &o_error) const;

	uint32_t getStopTime() const { return m_stopTime; }
	int getHostId() const { return m_hostId; }
	bool isHostId(const int value) const { return m_hostId == value; }

	const std::string &getTaskName() const
	{
		if (m_exec)
			return m_exec->getName();
		else
			return ms_no_name;
	}

	/// Calculate memory totally allocated by class instance
	int calcWeight() const;

	/// Stop runnig task. Request from remote render host to stop it. Host will send message with new status
	/// back to finish session. If stopping from GUI, state can be provided, for example to
	/// AFJOB::STATE_SKIPPED_MASK to skip task.
	virtual void stop(const std::string &message, RenderContainer *renders, MonitorContainer *monitoring,
					  uint32_t i_state = 0);

  protected:
	/// Finish running task session. Release task from render and became a zombie (ready to be deleted).
	virtual void finish(const std::string &message, RenderContainer *renders, MonitorContainer *monitoring);

  protected:
	Task *m_task;
	Block *m_block;
	af::TaskExec *m_exec;
	af::TaskProgress *m_progress;
	int m_tasknum;
	int m_hostId; ///< Task Host Id
	int64_t *m_running_capacity_counter;

  private:
	int32_t *m_running_tasks_counter;

	uint32_t m_stopTime; ///< Time, when running task was asked to stop.
	bool m_zombie;

	static std::string ms_no_name;
};
