/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

/*
	Jobs server container.
*/
#include "jobcontainer.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mcafnodes.h"

#include "afcommon.h"
#include "branchsrv.h"
#include "branchescontainer.h"
#include "monitorcontainer.h"
#include "rendercontainer.h"
#include "useraf.h"
#include "usercontainer.h"

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

af::Msg * JobContainer::registerJob(JSON & i_object, BranchesContainer * i_branches, UserContainer * i_users, MonitorContainer * i_monitors)
{
	JobAf * job = new JobAf(i_object);

	int32_t id = 0; int64_t serial = 0; std::string err;
	if (registerJob(job, err, i_branches, i_users, i_monitors))
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

	if (err.size())
	{
		oss << ",\n\"error\":\"" << err << "\"";
	}
	else if (job == NULL)
	{
		oss << ",\n\"error\":\"Job registration failed. See server log for details.\"";
	}


	oss << "\n}";

	return af::jsonMsg(oss);
}

bool JobContainer::registerJob(JobAf * i_job, std::string & o_err, BranchesContainer * i_branches, UserContainer * i_users, MonitorContainer * i_monitors)
{
	if (i_job == NULL)
	{
		AF_ERR << "JobContainer::registerJob: Can't allocate memory for a new job.";
		return false;
	}

	if (i_users == NULL)
	{
		AF_ERR << "JobContainer::registerJob: Users container is not set.";
		delete i_job;
		return false;
	}

	// Job from store is already checked for validness
	if ((i_job->isFromStore() == false) && (i_job->isValidConstructed() == false))
	{
		o_err = "Invalid job.";
		delete i_job;
		return false;
	}


	// Process job banch and user
	BranchSrv * branch;
	UserAf * user;
	{
		// Lock containers:
		AfContainerLock bLock(i_branches, AfContainerLock::WRITELOCK);
		AfContainerLock jLock(this,       AfContainerLock::WRITELOCK);
		AfContainerLock uLock(i_users,    AfContainerLock::WRITELOCK);
		if (i_monitors)
			AfContainerLock mLock( i_monitors, AfContainerLock::WRITELOCK);

		// Process branch:
		AF_DEBUG << "JobContainer::registerJob: Checking job branch: " << i_job->getBranch();
		branch = i_branches->addBranchFromPath(i_job->getBranch(), i_monitors);
		if (branch == NULL)
		{
			delete i_job;
			o_err = "JobContainer::registerJob: Can't process job branch.";
			return false;
		}

		// Process user:
		AF_DEBUG << "JobContainer::registerJob: Checking job user: " << i_job->getUserName();
		user = i_users->addUser(i_job->getUserName(), i_job->getHostName(), i_monitors);
		if (user == NULL)
		{
			delete i_job;
			o_err = "JobContainer::registerJob: Can't register new user.";
			return false;
		}

		// Add job node to container.
		if (add(i_job) == false)
		{
			delete i_job;
			o_err = "JobContainer::registerJob: Can't add job to container.";
			return false;
		}

		// Locking nodes and adding job to branch and user
		AF_DEBUG << "JobContainer::registerJob: locking job.";
		i_job->lock();

		AF_DEBUG << "JobContainer::registerJob: locking branch.";
		branch->lock();
		branch->addJob(i_job);

		AF_DEBUG << "JobContainer::registerJob: locking user.";
		user->lock();
		user->addJob(i_job);

		// Adding monitoring events:
		if (i_monitors)
		{
			AF_DEBUG << "JobContainer::registerJob: monitor new job events.";
			i_monitors->addJobEvent( af::Monitor::EVT_jobs_add, i_job->getId(), user->getId());
			i_monitors->addEvent( af::Monitor::EVT_branches_change, branch->getId());
			i_monitors->addEvent( af::Monitor::EVT_users_change, user->getId());
		}
	}


	// initialize job ( create tasks output root_dir, execute "pre"commands if any)
	AF_DEBUG << "JobContainer::registerJob: initiaizing new job with user.";
	if (i_job->initialize() == false)
	{
		AF_DEBUG << "JobContainer::registerJob: Job initialization failed.";

		if (i_monitors)
		{
			AfContainerLock mLock( i_monitors, AfContainerLock::WRITELOCK);
			i_monitors->addJobEvent( af::Monitor::EVT_jobs_del, i_job->getId(), user->getId());
			i_monitors->addEvent( af::Monitor::EVT_users_change, user->getId());
			i_monitors->addEvent( af::Monitor::EVT_branches_change, user->getId());
		}

		{   // Set job to zombie:
			AfContainerLock bLock( i_branches, AfContainerLock::WRITELOCK);
			AfContainerLock jLock( this,       AfContainerLock::WRITELOCK);
			AfContainerLock uLock( i_users,    AfContainerLock::WRITELOCK);
			branch->unLock();
			user->unLock();
			i_job->deleteNode( NULL, NULL);
		}

		return false;
	}


	if (i_monitors)
	{
		AfContainerLock mLock( i_monitors, AfContainerLock::WRITELOCK);

		AF_DEBUG << "JobContainer::registerJob: monitor unlock job and user events.";
		i_monitors->addJobEvent( af::Monitor::EVT_jobs_change, i_job->getId(), user->getId());
		i_monitors->addEvent( af::Monitor::EVT_branches_change, branch->getId());
		i_monitors->addEvent( af::Monitor::EVT_users_change, user->getId());
	}

	// Unlocking nodes:
	{
		AF_DEBUG << "JobContainer::registerJob: unlocking user and job.";
		AfContainerLock bLock( i_branches, AfContainerLock::WRITELOCK);
		AfContainerLock jLock( this,       AfContainerLock::WRITELOCK);
		AfContainerLock uLock( i_users,    AfContainerLock::WRITELOCK);
		branch->unLock();
		user->unLock();
		i_job->unLock();
	}

	AFCommon::QueueLog("Job registered: " + i_job->v_generateInfoString());

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
