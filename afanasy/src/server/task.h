#pragma once

#include "../include/afjob.h"

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

	virtual void v_start( af::TaskExec * taskexec, int * runningtaskscounter, RenderAf * render, MonitorContainer * monitoring);

/// Update task state.
	virtual void v_updateState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring, bool & errorHost);

	virtual void v_refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring, int & errorHostId);

	void restart( bool onlyRunning, const std::string & message, RenderContainer * renders, MonitorContainer * monitoring);
	void restartError( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring);
	void skip( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring);

	virtual void v_appendLog( const std::string  & message);
	inline const std::list<std::string> & getLog() { return m_logStringList; }

	void errorHostsAppend( const std::string & hostname);
	bool avoidHostsCheck( const std::string & hostname) const;
	void getErrorHostsList( std::list<std::string> & o_list) const;
	const std::string getErrorHostsListString() const;
	inline void errorHostsReset() { m_errorHosts.clear(); m_errorHostsCounts.clear(); m_errorHostsTime.clear();}

	int calcWeight() const;
	int logsWeight() const;
	int blackListWeight() const;

	virtual void v_writeTaskOutput( const af::MCTaskUp & taskup) const;  ///< Write task output in tasksOutputDir.
	virtual void v_monitor( MonitorContainer * monitoring) const;
	virtual void v_updateDatabase() const;
	virtual const std::string v_getInfo( bool full = false) const;

	const std::string getOutputFileName( int startcount) const;

/// Construct message for request output from render if task is running, or filename to read output from, if task is not running.
	af::Msg * getOutput( int i_startcount, RenderContainer * i_renders, std::string & o_filename, std::string & o_error) const;

	void listenOutput( af::MCListenAddress & mclisten, RenderContainer * renders);

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
	void deleteRunningZombie();

private:
	int m_number;

	TaskRun * m_run;

	std::list<std::string>  m_errorHosts;       ///< Avoid error hosts list.
	std::list<int>          m_errorHostsCounts; ///< Number of errors on error host.
	std::list<time_t>       m_errorHostsTime;   ///< Time of the last error
};
