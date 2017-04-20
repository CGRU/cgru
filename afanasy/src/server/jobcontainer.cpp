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
#include "../libafanasy/logger.h"

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

void JobContainer::reconnectTask( af::TaskExec * i_taskexec, RenderAf & i_render, MonitorContainer * i_monitoring)
{
	#ifdef AFOUTPUT
	AF_DEBUG << "Reconnecting task " << i_taskexec << " with " << i_render;
	#endif

	JobContainerIt jobsIt( this);
	JobAf* job = jobsIt.getJob( i_taskexec->getJobId());
	if( NULL == job )
	{
		AF_ERR << "Job with id=" << i_taskexec->getJobId() << " does not exists.";
		delete i_taskexec;
		return;
	}

	return job->reconnectTask( i_taskexec, i_render, i_monitoring);
}

af::Msg * JobContainer::registerJob( JSON & i_object, UserContainer * i_users, MonitorContainer * i_monitoring)
{
	JobAf * job = new JobAf( i_object);

	int32_t id = 0; int64_t serial = 0; std::string err;
	if( registerJob( job, err, i_users, i_monitoring))
	{
		id = job->getId();
		serial = job->getSerial();
	}
	else
		job = NULL;

	std::ostringstream oss;
	oss << "{";
	oss << "\n\"id\":" << id;
	oss << ",\n\"serial\":" << serial;

	if( err.size())
	{
		oss << ",\n\"error\":\"" << err << "\"";
	}
	else if( job == NULL )
	{
		oss << ",\n\"error\":\"Job registration failed. See server log for details.\"";
	}


	oss << "\n}";

	return af::jsonMsg( oss);
}

bool JobContainer::registerJob( JobAf *job, std::string & o_err, UserContainer *users, MonitorContainer * monitoring)
{
	if( job == NULL )
	{
		AF_ERR << "JobContainer::registerJob: Can't allocate memory for a new job.";
		return false;
	}

	if( users == NULL )
	{
		AF_ERR << "JobContainer::registerJob: Users container is not set.";
		delete job;
		return false;
	}

	// Job from store is already checked for validness
	if(( job->isFromStore() == false ) && (job->isValidConstructed() == false ))
	{
		o_err = "Invalid job.";
		delete job;
		return false;
	}

	UserAf *user;

	{  // Register new user if job has a new user name.
		AfContainerLock jLock( this, AfContainerLock::WRITELOCK);
		AfContainerLock uLock( users, AfContainerLock::WRITELOCK  );

		if( monitoring) AfContainerLock mLock( monitoring, AfContainerLock::WRITELOCK  );

		AF_DEBUG << "JobContainer::registerJob: Checking job user: " << job->getUserName().c_str();
		user = users->addUser( job->getUserName(), job->getHostName(), monitoring);
		if( user == NULL )
		{
			delete job;
			o_err = "JobContainer::registerJob: Can't register new user.";
			return false;
		}

		// Add job node to container.
		if( add( job) == false )
		{
			delete job;
			o_err = "JobContainer::registerJob: Can't add job to container.";
			return false;
		}
		AF_DEBUG << "JobContainer::registerJob: locking job.";
		job->lock();

		AF_DEBUG << "JobContainer::registerJob: locking user.";
		user->lock();
		user->addJob( job);
		if( monitoring )
		{
			AF_DEBUG << "JobContainer::registerJob: monitor new job events.";
			monitoring->addJobEvent( af::Monitor::EVT_jobs_add, job->getId(), user->getId());
			monitoring->addEvent( af::Monitor::EVT_users_change, user->getId());
		}
	}

	// initialize job ( create tasks output root_dir, execute "pre"commands if any)
	AF_DEBUG << "JobContainer::registerJob: initiaizing new job with user.";
	if( job->initialize() == false)
	{
		AF_DEBUG << "JobContainer::registerJob: Job initialization failed.";

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

		return false;
	}

	if( monitoring )
	{
		AfContainerLock mLock( monitoring,  AfContainerLock::WRITELOCK  );

		AF_DEBUG << "JobContainer::registerJob: monitor unlock job and user events.";
		monitoring->addJobEvent( af::Monitor::EVT_jobs_change, job->getId(), user->getId());
		monitoring->addEvent( af::Monitor::EVT_users_change, user->getId());
	}

	AF_DEBUG << "JobContainer::registerJob: unlocking user and job.";

	{
		AfContainerLock jLock( this,  AfContainerLock::WRITELOCK);
		AfContainerLock uLock( users, AfContainerLock::WRITELOCK);
		user->unLock();
		job->unLock();
	}

	AFCommon::QueueLog("Job registered: " + job->v_generateInfoString());

	return true;
}

const std::vector<int32_t> JobContainer::getIdsBySerials( const std::vector<int64_t> & i_serials)
{
	std::vector<int32_t> ids;

	JobContainerIt jobsIt( this);
	for( JobAf *job = jobsIt.job(); job != NULL; jobsIt.next(), job = jobsIt.job())
	{
		for( int i = 0; i < i_serials.size(); i++)
		{
			if( i_serials[i] == job->getSerial())
			{
				ids.push_back( job->getId());
				break;
			}
		}
	}

	if( ids.size() == 0 )
		ids.push_back(-1);

	return ids;
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
