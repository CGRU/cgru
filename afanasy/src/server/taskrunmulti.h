#pragma once

#include "../libafanasy/name_af.h"

#include "taskrun.h"

/// Afanasy running multi host task class.
class TaskRunMulti : public TaskRun
{
  public:
	/// Constructor.
	/** Create a running task session handler class.
	*** Multihost task not starts in constructor, it adds a first host here, and then wait for others.
	**/
	TaskRunMulti(Task *i_runningTask, af::TaskExec *i_taskExec, af::TaskProgress *i_taskProgress,
				 Block *i_taskBlock, RenderAf *i_render, MonitorContainer *i_monitoring);

	~TaskRunMulti();

	/// Calculate memory totally allocated by class instance
	int calcWeight() const;

	/// Add a host. Start task if there is enought hosts quantity.
	void addHost(af::TaskExec *taskexec, RenderAf *render, MonitorContainer *monitoring);

	/// Update task state by sent message, almost often from remote render host
	virtual void update(const af::MCTaskUp &taskup, RenderContainer *renders, MonitorContainer *monitoring,
						bool &errorHost) override;

	/// Do some work every period of time. Return true if there are some changes for database and monitoring.
	virtual bool refresh(time_t currentTime, RenderContainer *renders, MonitorContainer *monitoring,
						 int &errorHostId) override;

	/// Multi hosts tast can be running but still ready (acception hosts).
	virtual int v_getRunningRenderID(std::string &o_error) const override;

	void stdOut(bool full = false) const;

	/// Stop runnig task. Request from remote render host to stop it. Host will send message with new status
	/// back to finish session.
	/** Stop slave hosts if they run any service.
	 **/
	virtual void stop(const std::string &message, RenderContainer *renders, MonitorContainer *monitoring,
					  uint32_t i_state = 0) override;

  protected:
	/// Catch master task finishing session. Launch multi host task stopping.
	virtual void finish(const std::string &message, RenderContainer *renders,
						MonitorContainer *monitoring) override;

  private:
	void setMasterTask();
	void startServices(RenderContainer *renders);
	void startMaster(RenderContainer *renders, MonitorContainer *monitoring);
	void releaseHost(RenderContainer *renders, MonitorContainer *monitoring,
					 const af::MCTaskUp *taskup = NULL);
	void setProgressHostame();
	void stopSlaveService(RenderContainer *renders, MonitorContainer *monitoring, const af::MCTaskUp &taskup);

  private:
	bool m_has_service;
	bool m_master_running;
	bool m_stopping;

	uint32_t m_time_last_host_added;
	uint32_t m_time_services_started;
	uint32_t m_time_services_stopped;

	std::string m_master_hostname;
	std::list<af::TaskExec *> m_execs;
	std::list<int> m_hostids;
	std::list<std::string> m_hostnames;
};
