#pragma once

#include "../include/afjob.h"

#include "../libafanasy/msgclasses/mctask.h"
#include "../libafanasy/name_af.h"
#include "../libafanasy/taskprogress.h"

class JobAf;
class RenderAf;
class Block;
class TaskRun;
class MonitorContainer;
class RenderContainer;

class Task
{
public:
	Task( Block * taskBlock, af::TaskProgress * taskProgress, int taskNumber);
	virtual ~Task();

public:

	inline int getNumber() const { return m_number;}

	af::TaskExec * genExec() const;

	/// This method taks the ownership of `taskexec`
	virtual void v_start( af::TaskExec * i_taskexec, RenderAf * i_render, MonitorContainer * i_monitoring, int32_t * io_running_tasks_counter, int64_t * io_running_capacity_counter);

	/// Reconnect Task to an existing TaskExec
	/// This method taks the ownership of `taskexec`
	void reconnect( af::TaskExec * i_taskexec, RenderAf * i_render, MonitorContainer * i_monitoring, int32_t * io_running_tasks_counter, int64_t * io_running_capacity_counter);

	/// Update task state.
	virtual void v_updateState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring, bool & errorHost);

	virtual void v_refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring, int & errorHostId);

	void restart( const std::string & i_message, RenderContainer * i_renders, MonitorContainer * i_monitoring, uint32_t i_state = 0);

	void skip( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring);
	
	virtual void v_appendLog( const std::string  & message);
	inline const std::list<std::string> & getLog() { return m_logStringList; }

	void errorHostsAppend( const std::string & hostname);
	bool avoidHostsCheck( const std::string & hostname) const;
	void getErrorHostsList( std::list<std::string> & o_list) const;
	inline void errorHostsReset() { m_errorHosts.clear(); m_errorHostsCounts.clear(); m_errorHostsTime.clear();}

	int calcWeight() const;
	int logsWeight() const;
	int blackListWeight() const;

	/// Store task output:
	/// Need to be virtual, as system job task output storing is not needed
	virtual void v_writeTaskOutput( const char * i_data, int i_size) const;

	virtual void v_monitor( MonitorContainer * monitoring) const;

	// Store function should be empty in system job tasks
	virtual void v_store();

	virtual const std::string v_getInfo( bool full = false) const;

	const std::string getOutputFileName( int i_starts_count) const;

	/// Set render id if task is running, or filename to read output from
	void getOutput( af::MCTask & io_mctask, std::string & o_error) const;

	af::Msg * getStoredFiles() const;
	void getStoredFiles( std::ostringstream & i_str) const;

	void listenOutput( RenderContainer * i_renders, bool i_subscribe);

	void stdOut( bool full = false) const;

	inline bool isReady()   const { return m_progress->state & AFJOB::STATE_READY_MASK;   }
	inline bool isRunning() const { return m_progress->state & AFJOB::STATE_RUNNING_MASK; }
	inline bool isDone()    const { return m_progress->state & AFJOB::STATE_DONE_MASK;    }
	inline bool isError()   const { return m_progress->state & AFJOB::STATE_ERROR_MASK;   }

	bool m_solved;

protected:
	af::TaskProgress * m_progress;
	std::list<std::string> m_logStringList;    ///< Task log.
	Block * m_block;

private:
	void initStoreFolders();
	void storeFiles( const af::MCTaskUp & i_taskup);
	void deleteRunningZombie();

private:
	int m_number;

	std::string m_store_dir;
	std::string m_store_dir_output;
	std::string m_store_dir_files;
	std::string m_store_file_progress;

	std::vector<std::string> m_stored_files;
	std::vector<std::string> m_parsed_files;

	TaskRun * m_run;

	std::list<std::string>  m_errorHosts;       ///< Avoid error hosts list.
	std::list<int>          m_errorHostsCounts; ///< Number of errors on error host.
	std::list<time_t>       m_errorHostsTime;   ///< Time of the last error

	int m_listen_count;
};

