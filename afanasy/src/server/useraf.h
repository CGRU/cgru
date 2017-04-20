#pragma once

#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mcafnodes.h"
#include "../libafanasy/user.h"

#include "aflist.h"
#include "afnodesolve.h"

class Action;
class JobAf;
class RenderAf;
class UserContainer;

/// Server side of Afanasy user.
class UserAf : public af::User, public AfNodeSolve
{
public:
	/// Create a new user. User on job creation by unknown user.
	UserAf( const std::string & username, const std::string & host);

	/// Construct job from JSON.
	UserAf( JSON & i_object);

	/// Create user from store.
	UserAf( const std::string & i_store_dir);

	~UserAf();

	bool initialize();

	void addJob(    JobAf * i_job);     ///< Add job to user.
	void removeJob( JobAf * i_job);     ///< Remove job from user.

	/// Whether the user can produce a task
	/** Used to limit nodes for heavy solve algorithm **/
	bool v_canRun();

	/// Whether the user can produce a task
	/** Used to limit nodes for heavy solve algorithm **/
	bool v_canRunOn( RenderAf * i_render);

	/// Generate task for \c render from list, return \c render if task generated or NULL.
	virtual RenderAf * v_solve( std::list<RenderAf*> & i_renders_list, MonitorContainer * i_monitoring); 

	void jobsinfo( af::MCAfNodes &mcjobs); ///< Generate all uses jobs information.
	
	bool getJobs( std::ostringstream & o_str);

	void v_refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);///< Refresh user attributes corresponding on jobs information.

	virtual void v_action( Action & i_action);

	virtual int v_calcWeight() const; ///< Calculate and return memory size.

	inline AfList * getJobsList() { return &m_jobslist; }

	inline const std::vector<int32_t> generateJobsIds() const { return m_jobslist.generateIdsList();}

	void jobPriorityChanged( JobAf * i_job, MonitorContainer * i_monitoring);

	af::Msg * writeJobdsOrder( bool i_binary) const;

	/// Set container.
	inline static void setUserContainer( UserContainer * i_users ) { ms_users = i_users;}

	void logAction( const Action & i_action, const std::string & i_node_name);

protected:
	void v_calcNeed();

private:
	bool refreshCounters();

	void updateJobsOrder( af::Job * newJob = NULL);

	void deleteNode( MonitorContainer * i_monitoring);

private:
	AfList m_jobslist; ///< Jobs list.

private:
   static UserContainer * ms_users;
};
