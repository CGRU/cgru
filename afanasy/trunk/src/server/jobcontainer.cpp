#include "jobcontainer.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "../libafanasy/msgclasses/mcafnodes.h"

#include "../libafsql/qdbconnection.h"

#include "afcommon.h"
#include "msgaf.h"
#include "useraf.h"
#include "usercontainer.h"
#include "rendercontainer.h"
#include "monitorcontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

afsql::DBConnection * JobContainer::afDB = NULL;

JobContainer::JobContainer():
   AfContainer( "Jobs", AFJOB::MAXQUANTITY)
{
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
      job->updateTaskState( taskup, renders, monitoring);
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

   if( job->isConstructed() == false )
   {
      AFERROR("JobContainer::job_register: Job is not constructed.")
      return 0;
   }

   if( users == NULL )
   {
      AFERROR("JobContainer::job_register: Users container is not set.")
      return 0;
   }

   // Check database connection if it is working and if it is a new job:
   if( afDB->isWorking() && ( job->fromDataBase() == false ))
   {
      afsql::DBConnection dbcheck("AFANASY_check");
      if( false == dbcheck.isWorking())
      {
         std::string str("ALARM! Server database connection error. Contact your system administrator.");
         AFCommon::QueueLog( str);
         AfContainerLock mLock( monitoring, AfContainer::WRITELOCK);
         monitoring->sendMessage( str);
         return 0;
      }
   }

   UserAf *user;

   {  // Register new user if job has a new user name.
      AfContainerLock uLock( users, AfContainer::WRITELOCK  );
      if( monitoring) AfContainerLock mLock( monitoring, AfContainer::WRITELOCK  );

      AFINFA("JobContainer::job_register: Checking job user '%s'", job->getUserName().toUtf8().data())
      user = users->addUser( job->getUserName(), job->getHostName(), monitoring);
      if( user == NULL )
      {
         delete job;
         AFERROR("JobContainer::job_register: Can't register new user.")
         return 0;
      }
      {  // Add job node to container.
         AfContainerLock jLock( this, AfContainer::WRITELOCK);

         if( add((af::Node*)job) == false )
         {
            delete job;
            AFERROR("JobContainer::job_register: Can't add job to container.")
            return 0;
         }
         AFINFO("JobContainer::job_register: locking job.")
         job->lock();
      }
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
   if( job->initialize( user) == false)
   {
      AFERROR("JobContainer::job_register: Job initialization failed.")

      if( monitoring )
      {
         AfContainerLock mLock( monitoring,  AfContainer::WRITELOCK  );

         monitoring->addJobEvent( af::Msg::TMonitorJobsDel, job->getId(), user->getId());
         monitoring->addEvent( af::Msg::TMonitorUsersChanged, user->getId());
      }

      user->unLock();
      job->unLock();

      job->setZombie( NULL, NULL);

      return 0;
   }

   // write job to database ( if it is not created from database)
   if( job->fromDataBase() == false )
   {
      AFINFO("JobContainer::job_register: writing job to database.")
      if( afDB == NULL )
      {
         AFERROR("JobContainer::job_register: Afanasy database is not set.")
      }
      else
      {
         if( afDB->DBOpen())
         {
            afDB->addJob( job);
            afDB->DBClose();
         }
      }
   }

   if( monitoring )
   {
      AfContainerLock mLock( monitoring,  AfContainer::WRITELOCK  );

      AFINFO("JobContainer::job_register: monitor unlock job and user events.")
      monitoring->addJobEvent( af::Msg::TMonitorJobsChanged, job->getId(), user->getId());
      monitoring->addEvent( af::Msg::TMonitorUsersChanged, user->getId());
   }

   AFCommon::QueueLog("Job registered: " + job->generateInfoString());
   int newJobId = job->getId();

   AFINFO("JobContainer::job_register: unlocking user and job.")
   user->unLock();
   job->unLock();

   return newJobId;
}

void JobContainer::getWeight( af::MCJobsWeight & jobsWeight )
{
   JobContainerIt jobsIt( this);
   for( JobAf *job = jobsIt.job(); job != NULL; jobsIt.next(), job = jobsIt.job())
   {
      int weight = job->calcWeight();
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
