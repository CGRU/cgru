#pragma once

#include "../include/afjob.h"

#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mcjobsweight.h"

#include "../libafsql/name_afsql.h"

#include "afcontainer.h"
#include "afcontainerit.h"
#include "jobaf.h"

class MsgAf;
class UserContainer;

/// All Afanasy jobs store in this container.
class JobContainer: public AfContainer
{
public:
	JobContainer();
	~JobContainer();

	/// Register a new job, new id returned on success, else return 0.
	int job_register( JobAf *job, UserContainer *users, MonitorContainer * monitoring);

	/// Update some task state of some job.
	void updateTaskState( af::MCTaskUp &taskup, RenderContainer * renders, MonitorContainer * monitoring);

	void getWeight( af::MCJobsWeight & jobsWeight );
};

//########################## Iterator ##############################

/// Afanasy jobs interator.
class JobContainerIt: public AfContainerIt
{
public:
	JobContainerIt( JobContainer* jobContainer, bool skipZombies = true);
	~JobContainerIt();

	inline JobAf * job() { return (JobAf*)(getNode()); }
	inline JobAf * getJob( int id) { return (JobAf*)(get( id)); }

private:
};
