#pragma once

#include "../libafanasy/name_af.h"
#include "../libafanasy/job.h"
#include "../libafanasy/msgclasses/mctaskup.h"
#include "../libafanasy/msgclasses/mctaskpos.h"
#include "../libafanasy/msgclasses/mctaskspos.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "afnodesrv.h"
#include "listeners.h"

class Action;
class Block;
class JobContainer;
class JobPy;
class MsgAf;
class RenderAf;
class RenderContainer;
class Task;
class UserAf;

/// Server side of Afanasy job.
class JobAf : public af::Job , public AfNodeSrv
{
public:
/// Construct job from JSON.
	JobAf( JSON & i_object);

/// Construct empty Job for store.
	JobAf( const std::string & i_store_dir = "");

	virtual ~JobAf();

	bool isValidConstructed() const;

    virtual void v_setZombie( RenderContainer * renders, MonitorContainer * monitoring);        ///< Set job node to zombie.

	void writeProgress( af::Msg &msg);   ///< Write job progress in message.

	af::Msg * writeProgress( bool json);   ///< Write job progress in message.

	af::Msg * writeBlocks( std::vector<int32_t> i_block_ids, std::vector<std::string> i_modes) const;

	af::Msg * writeTask( int i_b, int i_t, const std::string & i_mode) const;

	af::Msg * writeErrorHosts() const;
	af::Msg * writeErrorHosts( int b, int t) const;
	void writeErrorHosts( std::list<std::string> & o_list) const;

    const std::string v_getErrorHostsListString() const; /// Get avoid hosts list.
    virtual const std::string v_getErrorHostsListString( int b, int t) const; /// Get avoid hosts list for \c t task in \c b block.

/// Get \c task task from \c block log.
	const std::list<std::string> & getTaskLog( int block, int task) const;

	af::TaskExec * generateTask( int block, int task) const;

	const std::string generateTaskName( int i_b, int i_t) const;

/// Construct message for retrieveing output from running remote host or filename if task is not running.
/** Virtual for system job, it just sets an error that output is not available.**/
	virtual af::Msg * v_getTaskStdOut( int i_b, int i_t, int i_n, RenderContainer * i_renders,
		std::string & o_filename, std::string & o_error) const;

/// Whether the job can produce a task
/** Used to limit nodes for heavy solve algorithm **/
     bool v_canRun();

/// Whether the job can produce a task
/** Used to limit nodes for heavy solve algorithm **/
     bool v_canRunOn( RenderAf * i_render);

/// Solve a job. Job send ready task to Render, if any.
    virtual bool v_solve( RenderAf *render, MonitorContainer * monitoring);

/// Update task state.
    virtual void v_updateTaskState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring);

/// Send tasks output to a specified address.
	void listenOutput( af::MCListenAddress & mclisten, RenderContainer * renders);

	void skipTasks(    const af::MCTasksPos & taskspos, RenderContainer * renders, MonitorContainer * monitoring);  ///< Skip some tasks.
    virtual void v_restartTasks( const af::MCTasksPos & taskspos, RenderContainer * renders, MonitorContainer * monitoring);  ///< Restart some tasks.

/// Refresh job. Calculate attributes from tasks progress.
    virtual void v_refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);

	virtual void v_action( Action & i_action);

	void setUser( UserAf * i_user);

/// Initialize new job, came to Afanasy container.
	bool initialize();

	int getUid() const;

	virtual int v_calcWeight()          const;  ///< Calculate and return memory size.
	inline  int getLogsWeight()       const { return m_logsWeight;}        ///< Get job and all its tasks logs weight ( this value is stored after \c calcWeight method).
	inline  int getBlackListsWeight() const { return m_blackListsWeight;}  ///< Get job and all its tasks black lists weight ( this value is stored after \c calcWeight method).
	inline  int getProgressWeight()   const { return progressWeight;}    ///< Get job tasks progress ( this value is stored after \c calcWeight method).

	bool checkBlockTaskNumbers( int BlockNum, int TaskNum, const char * str = NULL) const;

	void setUserListOrder( int index, bool updateDtabase);

	void addRenderCounts( RenderAf * render);
	int  getRenderCounts( RenderAf * render) const;
	void remRenderCounts( RenderAf * render);

	const std::string & getTasksProgessDir() const { return m_store_dir_progress; }
	const std::string & getTasksOuputDir() const { return m_store_dir_output; }

public:
	/// Set Jobs Container.
	inline static void setJobContainer( JobContainer *Jobs){ ms_jobs = Jobs;}

protected:
	/// Allocate JobInfo, tasksLog.
	void construct();

    virtual Block * v_newBlock( int numBlock); ///< Virtual function to create system blocks in a system job

    void v_calcNeed();

protected:
	af::JobProgress * m_progress;    ///< Tasks progress.
	Block ** m_blocks;              ///< Blocks.

private:
	bool m_deletion;                ///< Whether the job is deleting.

	std::list<RenderAf*> renders_ptrs;
	std::list<int> renders_counts;

	Listeners listeners;     ///> Addresses to send task output to.

	UserAf * m_user;

	std::string m_store_dir_progress;   ///< Store directory.
	std::string m_store_dir_output;   ///< Store directory.

private:
	mutable int progressWeight;
	mutable int m_logsWeight;
	mutable int m_blackListsWeight;

private:
	void initializeValues();
	void initStoreDirs();

	virtual void v_priorityChanged( MonitorContainer * i_monitoring);

/// Skip or restart some tasks.
	void tasks_Skip_Restart( const af::MCTasksPos &taskspos, bool restart, RenderContainer * renders, MonitorContainer * monitoring);

/// Check whether job has not done depend jobs.
/** If \c onlyIfDepeds == \c true , check process only if job is already waiting for others. **/
	void checkDepends();

/// Restart tasks, can restart only running tasks.
	void restartAllTasks( bool onlyRunning, const std::string & message, RenderContainer * renders, MonitorContainer * monitoring);

/// Restart tasks with errors
	void restartErrors( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring);

	af::TaskExec *genTask( RenderAf *render, int block, int task, std::list<int> * blocksIds, MonitorContainer * monitoring);

private:
	static JobContainer * ms_jobs;          ///< Jobs container pointer.
};
