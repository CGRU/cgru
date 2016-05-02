#include "jobcontainer.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mcafnodes.h"

#include "afcommon.h"
#include "useraf.h"
#include "usercontainer.h"
#include "rendercontainer.h"
#include "monitorcontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

JobContainer::JobContainer():
    AfContainer( "Jobs", AFJOB::MAXQUANTITY)
{
	JobAf::setJobContainer( this);
}

JobContainer::~JobContainer()
{
AFINFO("JobContainer::~JobContainer:")
}

void JobContainer::updateTaskState( af::MCTaskUp &taskup, RenderContainer * renders, MonitorContainer * monitoring)
{
    switch( taskup.getStatus())
    {
    case af::TaskExec::UPNULL:
    case af::TaskExec::UPNoTaskRunning:
    case af::TaskExec::UPNoJob:
    case af::TaskExec::UPLAST:
        AFERRAR("JobContainer::updateTaskState: Bad task update status (jobID=%d).\n", taskup.getNumJob());
    return;
    }

    JobContainerIt jobsIt( this);
    JobAf* job = jobsIt.getJob( taskup.getNumJob());
    if( job != NULL )
    {
        job->v_updateTaskState( taskup, renders, monitoring);
        return;
    }

    // Job does not exist!
    AFERRAR("JobContainer::updateTaskState: Job with id=%d does not exists.", taskup.getNumJob())
    if( taskup.getStatus() == af::TaskExec::UPPercent) RenderAf::closeLostTask( taskup);
}

int JobContainer::job_register( JobAf *job, UserContainer *users, MonitorContainer * monitoring)
{
//printf("Registering new job:\n");

	if( users == NULL )
	{
		AFERROR("JobContainer::job_register: Users container is not set.")
		delete job;
		return 0;
	}

	if( job == NULL )
	{
		AFERROR("JobContainer::job_register: Can't allocate memory for new job.")
		return 0;
	}

	// Job from sore already checked for validness during afret reading files
	if(( job->isFromStore() == false ) && (job->isValidConstructed() == false ))
	{
		AFCommon::QueueLogError("Deleting invalid job.");
		delete job;
		return 0;
	}

	UserAf *user;

	{  // Register new user if job has a new user name.
		AfContainerLock jLock( this, AfContainerLock::WRITELOCK);
		AfContainerLock uLock( users, AfContainerLock::WRITELOCK  );

		if( monitoring) AfContainerLock mLock( monitoring, AfContainerLock::WRITELOCK  );

		//AFINFA("JobContainer::job_register: Checking job user '%s'", job->getUserName().toUtf8().data())
		AFINFA( "JobContainer::job_register: Checking job user '%s'", job->getUserName().c_str() )
		user = users->addUser( job->getUserName(), job->getHostName(), monitoring);
		if( user == NULL )
		{
			delete job;
			AFERROR("JobContainer::job_register: Can't register new user.")
			return 0;
		}

		// Add job node to container.
		if( add( job) == false )
		{
			delete job;
			AFERROR("JobContainer::job_register: Can't add job to container.")
			return 0;
		}
		AFINFO("JobContainer::job_register: locking job.")
		job->lock();

		AFINFO("JobContainer::job_register: locking user.")
		user->lock();
		user->addJob( job);
		if( monitoring )
		{
			AFINFO("JobContainer::job_register: monitor new job events.")
			monitoring->addJobEvent( af::Monitor::EVT_jobs_add, job->getId(), user->getId());
			monitoring->addEvent( af::Monitor::EVT_users_change, user->getId());
		}
	}

	// initialize job ( create tasks output root_dir, execute "pre"commands if any)
	AFINFO("JobContainer::job_register: initiaizing new job with user.")
	if( job->initialize() == false)
	{
		AFERROR("JobContainer::job_register: Job initialization failed.")

		if( monitoring )
		{
			AfContainerLock mLock( monitoring,  AfContainerLock::WRITELOCK  );

			monitoring->addJobEvent( af::Monitor::EVT_jobs_del, job->getId(), user->getId());
			monitoring->addEvent( af::Monitor::EVT_users_change, user->getId());
		}

		{   // Set job to zombie:
			AfContainerLock jLock( this, AfContainerLock::WRITELOCK);
			AfContainerLock uLock( users, AfContainerLock::WRITELOCK);
			user->unLock();
			job->deleteNode( NULL, NULL);
		}

		return 0;
	}

	if( monitoring )
	{
		AfContainerLock mLock( monitoring,  AfContainerLock::WRITELOCK  );

		AFINFO("JobContainer::job_register: monitor unlock job and user events.")
		monitoring->addJobEvent( af::Monitor::EVT_jobs_change, job->getId(), user->getId());
		monitoring->addEvent( af::Monitor::EVT_users_change, user->getId());
	}

	AFCommon::QueueLog("Job registered: " + job->v_generateInfoString());
	int newJobId = job->getId();

	AFINFO("JobContainer::job_register: unlocking user and job.")

	{
		AfContainerLock jLock( this,  AfContainerLock::WRITELOCK);
		AfContainerLock uLock( users, AfContainerLock::WRITELOCK);
		user->unLock();
		job->unLock();
	}

	return newJobId;
}

void JobContainer::getWeight( af::MCJobsWeight & jobsWeight )
{
   JobContainerIt jobsIt( this);
   for( JobAf *job = jobsIt.job(); job != NULL; jobsIt.next(), job = jobsIt.job())
   {
      int weight = job->v_calcWeight();
      jobsWeight.add(
            job->getName(),
            job->getId(),
            job->getLogsWeight(),
            job->getBlackListsWeight(),
            job->getProgressWeight(),
            weight
         );
   }
}

bool JobContainer::solve( RenderAf * i_render, MonitorContainer * i_monitoring)
{
    AfList jobList;

    JobContainerIt jobsIt( this);
    for( JobAf *job = jobsIt.job(); job != NULL; jobsIt.next(), job = jobsIt.job())
        jobList.add( job);

    return jobList.solve(af::Node::SolveByPriority, i_render, i_monitoring);
}


//############################################################################
//                               JobQueueIt
//############################################################################

JobContainerIt::JobContainerIt( JobContainer* jobContainer, bool skipZombies):
   AfContainerIt( (AfContainer*)jobContainer, skipZombies)
{
}

JobContainerIt::~JobContainerIt()
{
}
