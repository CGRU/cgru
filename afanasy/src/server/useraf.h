#pragma once

#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mcafnodes.h"

#include "../libafsql/dbuser.h"

#include "aflist.h"
#include "afnodesrv.h"

class Action;
class JobAf;
class RenderAf;
class UserContainer;

/// Server side of Afanasy user.
class UserAf : public afsql::DBUser, public AfNodeSrv
{
public:
	/// Create a new user.
	UserAf( const std::string & username, const std::string & host);

	/// Create user from database.
	UserAf( int uid);

	~UserAf();

	void addJob(    JobAf * i_job);     ///< Add job to user.
	void removeJob( JobAf * i_job);     ///< Remove job from user.

	/// Whether the user can produce a task
	/** Used to limit nodes for heavy solve algorithm **/
	bool v_canRun();

	/// Whether the user can produce a task
	/** Used to limit nodes for heavy solve algorithm **/
	bool v_canRunOn( RenderAf * i_render);

	bool v_solve( RenderAf * i_render, MonitorContainer * i_monitoring); ///< Generate task for \c render host, return \c true if task generated.

	void jobsinfo( af::MCAfNodes &mcjobs); ///< Generate all uses jobs information.
	
	bool getJobs( std::ostringstream & o_str);

	void v_refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);///< Refresh user attributes corresponding on jobs information.

	virtual void v_action( Action & i_action);

	virtual int v_calcWeight() const; ///< Calculate and return memory size.

	inline AfList * getJobsList() { return &m_jobslist; }

	inline const std::vector<int32_t> generateJobsIds() const { return m_jobslist.generateIdsList();}

	af::Msg * writeJobdsOrder() const;

	/// Set container.
	inline static void setUserContainer( UserContainer * i_users ) { ms_users = i_users;}

protected:
	void v_calcNeed();

private:
	void construct();
	void updateJobsOrder( af::Job * newJob = NULL);
	void v_setZombie( MonitorContainer * i_monitoring);    ///< Set user to zombie.
	virtual void v_priorityChanged( MonitorContainer * i_monitoring);

private:
	uint32_t m_zombietime; ///< User zombie time - time to have no jobs before deletion.

	AfList m_jobslist; ///< Jobs list.

private:
   static UserContainer * ms_users;
};
