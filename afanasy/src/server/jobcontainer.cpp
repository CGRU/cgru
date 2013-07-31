#include "jobcontainer.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "../libafanasy/environment.h"

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

	if( job->isValidConstructed() == false )
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

const std::vector<int> JobContainer::getStoredIds()
{
	std::vector<int> ids;
	std::string folder = af::Environment::getJobsDir();

#ifdef WINNT
	HANDLE thousand_dir;
	WIN32_FIND_DATA thousand_data;
	if(( thousand_dir = FindFirstFile(( folder + "\\*").c_str(), &thousand_data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			std::string thousand( thousand_data.cFileName);
			if( thousand.find(".") == 0 ) continue;
			thousand = folder + '\\' + thousand;
			if( false == af::pathIsFolder( thousand)) continue;

			HANDLE id_dir;
			WIN32_FIND_DATA id_data;
			if(( id_dir = FindFirstFile(( thousand + "\\*").c_str(), &id_data)) != INVALID_HANDLE_VALUE)
			{
				do
				{
					std::string id( id_data.cFileName);
					if( thousand.find(".") == 0 ) continue;
					if( false == af::pathIsFolder( thousand + '\\' + id)) continue;
					ids.push_back( af::stoi( id));
				}
				while ( FindNextFile( id_dir, &id_data));

				FindClose( id_dir);
			}
			else
			{
				AFERRAR("JobContainer::getStoredIds: Can't open folder:\n%s", thousand.c_str())
				return ids;
			}

		} while ( FindNextFile( thousand_dir, &thousand_data));
		FindClose( thousand_dir);
	}
	else
	{
		AFERRAR("JobContainer::getStoredIds: Can't open folder:\n%s", folder.c_str())
		return ids;
	}

#else

	struct dirent * thousand_de = NULL;
	DIR * thousand_dir = opendir( folder.c_str());
	if( thousand_dir == NULL)
	{
		AFERRAR("JobContainer::getStoredIds: Can't open folder:\n%s", folder.c_str())
		return ids;
	}

	while( thousand_de = readdir( thousand_dir))
	{
		if( thousand_de->d_name[0] == '.' ) continue;
		std::string thousand = folder + '/' + thousand_de->d_name;
		if( false == af::pathIsFolder( thousand )) continue;

		struct dirent * id_de = NULL;
		DIR * id_dir = opendir( thousand.c_str());
		if( id_dir == NULL)
		{
			AFERRAR("JobContainer::getStoredIds: Can't open folder:\n%s", thousand.c_str())
			return ids;
		}

		while( id_de = readdir( id_dir))
		{
			if( id_de->d_name[0] == '.' ) continue;
			std::string id( id_de->d_name);
			if( false == af::pathIsFolder( thousand + '/' + id)) continue;
			ids.push_back( af::stoi( id));
		}
	}

	closedir(thousand_dir);
#endif

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
