#pragma once

#include "../include/afjob.h"

#include "../libafanasy/msgclasses/mctaskup.h"
#include "../libafanasy/render.h"
#include "../libafanasy/renderevents.h"
#include "../libafanasy/renderupdate.h"
#include "../libafanasy/taskexec.h"

#include "afnodesrv.h"

class Action;
class MsgQueue;
class JobContainer;
class RenderContainer;

/// Afanasy server side of Render host.
class RenderAf: public af::Render, public AfNodeSrv
{
public:
/// Construct Render from message and provided address.
	RenderAf( af::Msg * msg);
	~RenderAf();

/// Construct an offline render for store.
	RenderAf( const std::string & i_store_dir);

/// Set registration time ( and update time).
	void setRegistered();

/// Awake offline render
	void online( RenderAf * render, JobContainer * i_jobs, MonitorContainer * monitoring);

/// Add task \c taskexec to render, \c start or only capture it
/// Takes over the taskexec ownership
	void setTask( af::TaskExec *taskexec, MonitorContainer * monitoring, bool start = true);

/// Start tast \c taskexec on remote render host, task must be set before and exists on render.
	void startTask( af::TaskExec *taskexec);

/// Make Render to stop task.
	void stopTask( int jobid, int blocknum, int tasknum, int number);

	/// Make Render to stop task.
	inline void stopTask( const af::TaskExec * taskexec)
		{ stopTask(taskexec->getJobId(), taskexec->getBlockNum(), taskexec->getTaskNum(), taskexec->getNumber()); }

/// Make Render to stop task.
	inline void stopTask( const af::MCTaskUp &taskup)
		{ stopTask(taskup.getNumJob(), taskup.getNumBlock(), taskup.getNumTask(), taskup.getNumber());}

/// Make Render to finish task.
/// Releases the ownership of taskexec (Render will not own it any more)
	void taskFinished( const af::TaskExec * taskexec, MonitorContainer * monitoring);

/// Refresh parameters.
	void v_refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);

	af::Msg * writeTasksLog( bool i_binary);

/// Get host parameters from farm.
	bool getFarmHost( af::Host * newHost = NULL);

/// Deregister render, on SIGINT client recieving.
	void deregister( JobContainer * jobs, MonitorContainer * monitoring );

	virtual void v_action( Action & i_action);

	inline const std::list<std::string> & getTasksLog() { return m_tasks_log; }  ///< Get tasks log list.
	const std::string getServicesString() const;							 ///< Get services information.
	void jsonWriteServices( std::ostringstream & o_str) const; ///< Get services information.

	virtual int v_calcWeight() const; ///< Calculate and return memory size.

	bool canRunService( const std::string & type) const; ///< Check whether block can run a service

	// Update render and send instructions back:
	af::Msg * update( const af::RenderUpdate & i_up);

	// Called directly from solve cycle if it was not solved.
	void solvingFinished();

	// Need for server to write some farm parameters (gui double-click):
	af::Msg * writeFullInfo( bool binary) const;

	void wolWake(  MonitorContainer * i_monitoring, const std::string & i_msg = std::string());

	inline void addTaskOutput( const af::MCTaskPos & i_tp) { m_re.addTaskOutput( i_tp);}

	inline void listenTask( const af::MCTaskPos & i_tp, bool i_subscribe)
		{ if( i_subscribe) m_re.taskListenAdd( i_tp); else m_re.taskListenRem( i_tp); }

public:
	/// Set container:
	inline static void setRenderContainer( RenderContainer * i_container){ ms_renders = i_container;}

	/// Stop task on render if it is not running (for server)
	static void closeLostTask( const af::MCTaskUp &taskup);

private:
	void initDefaultValues();

	/// Add the task exec to this render and take over its ownership (meaning
	/// one should not free taskexec after having provided it to this method).
	void addTask( af::TaskExec * taskexec);
	/// Remove the task exec from this render and give back its ownership to the
	/// caller.
	void removeTask( const af::TaskExec * taskexec);

	void addService( const std::string & type);
	void remService( const std::string & type);

	void setService( const std::string & srvname, bool enable);
	void disableServices();

/// Stop tasks.
	void ejectTasks( JobContainer * jobs, MonitorContainer * monitoring, uint32_t upstatus, const std::string * i_keeptasks_username = NULL);

	void exitClient( const std::string & i_type, JobContainer * i_jobs, MonitorContainer * i_monitoring);	///< Exit Render client program.

	void launchAndExit( const std::string & i_cmd, bool i_exit, JobContainer * i_jobs, MonitorContainer * i_monitoring);

/// Set Render to Ofline. \c updateTaskState - whether to update it's state.
	void offline( JobContainer * jobs, uint32_t updateTaskState, MonitorContainer * monitoring, bool toZombie = false );

	void wolSleep( MonitorContainer * monitoring);

	void appendTasksLog( const std::string & message);  ///< Append tasks log with a \c message .

private:
	std::string m_farm_host_name;
	std::string m_farm_host_description;

	std::vector<int> m_services_counts;
	int m_services_num;

	std::vector<int> m_services_disabled_nums;

	std::list<std::string> m_tasks_log;							///< Tasks Log.

	af::RenderEvents m_re;

private:
	static RenderContainer * ms_renders;

};
