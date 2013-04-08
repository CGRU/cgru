#include "jobcontainer.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "../libafanasy/msgclasses/mcafnodes.h"

#include "../libafsql/dbconnection.h"

#include "afcommon.h"
#include "useraf.h"
#include "usercontainer.h"
#include "rendercontainer.h"
#include "monitorcontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

afsql::DBConnection * JobContainer::m_afDB = NULL;

JobContainer::JobContainer( afsql::DBConnection * i_afDB):
    AfContainer( "Jobs", AFJOB::MAXQUANTITY)
{
    m_afDB = i_afDB;
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

    if( job == NULL )
    {
        AFERROR("JobContainer::job_register: Can't allocate memory for new job.")
        return 0;
    }

	if( job->isValid() == false )
	{
		AFERROR("JobContainer::job_register: Job is invalid.")
		delete job;
		return 0;
	}

	if( job->isConstructed() == false )
	{
		AFERROR("JobContainer::job_register: Job is not constructed.")
		delete job;
		return 0;
	}

	if( users == NULL )
    {
        AFERROR("JobContainer::job_register: Users container is not set.")
		delete job;
		return 0;
    }

    // Check database connection if it is working and if it is a new job:
    if( m_afDB->isWorking() && ( job->fromDataBase() == false ))
    {
        afsql::DBConnection dbcheck("AFANASY_check");
        if( false == dbcheck.isWorking())
        {
            std::string str("ALARM! Server database connection error. Contact your system administrator.");
            AFCommon::QueueLog( str);
            AfContainerLock mLock( monitoring, AfContainerLock::WRITELOCK);
            monitoring->sendMessage( str);
            delete job;
            return 0;
        }
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
            monitoring->addJobEvent( af::Msg::TMonitorJobsAdd, job->getId(), user->getId());
            monitoring->addEvent( af::Msg::TMonitorUsersChanged, user->getId());
        }
    }

    // initialize job ( create tasks output folder, execute "pre"commands if any)
    AFINFO("JobContainer::job_register: initiaizing new job with user.")
    if( job->initialize() == false)
    {
        AFERROR("JobContainer::job_register: Job initialization failed.")

        if( monitoring )
        {
            AfContainerLock mLock( monitoring,  AfContainerLock::WRITELOCK  );

            monitoring->addJobEvent( af::Msg::TMonitorJobsDel, job->getId(), user->getId());
            monitoring->addEvent( af::Msg::TMonitorUsersChanged, user->getId());
        }

        {   // Set job to zombie:
            AfContainerLock jLock( this, AfContainerLock::WRITELOCK);
            AfContainerLock uLock( users, AfContainerLock::WRITELOCK);
            user->unLock();
            job->v_setZombie( NULL, NULL);
        }

        return 0;
    }

    // write job to database ( if it is not created from database)
    if( job->fromDataBase() == false )
    {
        AFINFO("JobContainer::job_register: writing job to database.")
        if( m_afDB == NULL )
        {
            AFERROR("JobContainer::job_register: Afanasy database is not set.")
        }
        else
        {
            if( m_afDB->DBOpen())
            {
                if( false == m_afDB->addJob( job))
                {
                    AFERRAR("Failed to add job to database: name = '%s', id = %d", job->getName().c_str(), job->getId())
                }
                m_afDB->DBClose();
            }
        }
    }

    if( monitoring )
    {
        AfContainerLock mLock( monitoring,  AfContainerLock::WRITELOCK  );

        AFINFO("JobContainer::job_register: monitor unlock job and user events.")
        monitoring->addJobEvent( af::Msg::TMonitorJobsChanged, job->getId(), user->getId());
        monitoring->addEvent( af::Msg::TMonitorUsersChanged, user->getId());
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
