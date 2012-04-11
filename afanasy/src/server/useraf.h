#pragma once

#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mcafnodes.h"

#include "../libafsql/dbuser.h"

#include "aflist.h"

class JobAf;
class RenderAf;

/// Server side of Afanasy user.
class UserAf : public afsql::DBUser
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
    bool canRun();

    /// Whether the user can produce a task
    /** Used to limit nodes for heavy solve algorithm **/
    bool canRunOn( RenderAf * i_render);

    bool solve( RenderAf * i_render, MonitorContainer * i_monitoring); ///< Generate task for \c render host, return \c true if task generated.

    void jobsinfo( af::MCAfNodes &mcjobs); ///< Generate all uses jobs information.
    
	bool getJobs( std::ostringstream & o_str);

    void refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);///< Refresh user attributes corresponding on jobs information.

    ///< Set some user attribute.
    bool action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring);

    void moveJobs( const af::MCGeneral & mcgeneral, int type);

    virtual int calcWeight() const; ///< Calculate and return memory size.

    inline AfList * getJobsList() { return &m_jobslist; }

    void generateJobsIds( af::MCGeneral & ids) const;

protected:
    void calcNeed();

private:
    void construct();
    void updateJobsOrder( af::Job * newJob = NULL);
    void setZombie( MonitorContainer * i_monitoring);    ///< Set user to zombie.

private:
    uint32_t m_zombietime; ///< User zombie time - time to have no jobs before deletion.

    AfList m_jobslist; ///< Jobs list.
};
