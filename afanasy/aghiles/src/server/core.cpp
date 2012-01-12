#include "core.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <memory.h>
#include <resolv.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "afcommon.h"
#include "msgaf.h"
#include "sysjob.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

extern Core* core;
//##############################################################################################

Core::Core():
   threadRun(      NULL ),
   threadReadMsg(  NULL ),
   msgQueue(      NULL ),
   talks(         NULL ),
   monitors(      NULL ),
   renders(       NULL ),
   jobs(          NULL ),
   users(         NULL ),
   afDB_JobRegister( "AFDB_JobRegister"),
   initialized( false)
{
   core = this;
   AFINFO("Core::Core:")

   // containers initialization
   jobs = new JobContainer();
   if( !jobs->isInitialized()) return;
   JobContainer::setDataBase( &afDB_JobRegister);
   JobAf::setJobContainer( jobs);

   users = new UserContainer();
   if( !users->isInitialized()) return;

   renders = new RenderContainer();
   if( !renders->isInitialized()) return;
   RenderAf::setRenderContainer( renders);

   talks = new TalkContainer();
   if( !talks->isInitialized()) return;

   monitors = new MonitorContainer();
   if( !monitors->isInitialized()) return;

   // check message queues initialization
   msgQueue = new MsgQueue("Run thread messages queue");
   if( !msgQueue->isInitialized()) return;

   // theads initialization
   ThreadPointers ptrs( this);
   threadRun = new ThreadRun( &ptrs);
   threadReadMsg = new ThreadReadMsg( &ptrs);


   bool hasSystemJob = false;
//
// Open database to get nodes:
//
   afDB_JobRegister.DBOpen();
   if( afDB_JobRegister.isOpen())
   {
      // Update database tables:
      afsql::UpdateTables( &afDB_JobRegister);

      //
      // Get Renders from database:
      //
      printf("Getting renders from database...\n");
      std::list<int> rids = afDB_JobRegister.getIntegers( afsql::DBRender::dbGetIDsCmd());
      for( std::list<int>::const_iterator it = rids.begin(); it != rids.end(); it++)
      {
         RenderAf * render = new RenderAf( *it);
         if( afDB_JobRegister.getItem( render)) renders->addRender( render);
         else delete render;
      }
      printf("%d renders founded.\n", (int)rids.size());

      //
      // Get Users from database:
      //
      printf("Getting users from database...\n");
      std::list<int> uids = afDB_JobRegister.getIntegers( afsql::DBUser::dbGetIDsCmd());
      for( std::list<int>::const_iterator it = uids.begin(); it != uids.end(); it++)
      {
         UserAf * user = new UserAf( *it);
         if( afDB_JobRegister.getItem( user)) users->addUser( user);
         else delete user;
      }
      printf("%d users founded.\n", (int)uids.size());

      //
      // Get Jobs from database:
      //
      printf("Getting jobs from database...\n");
      std::list<int> jids = afDB_JobRegister.getIntegers( afsql::DBJob::dbGetIDsCmd());
      for( std::list<int>::const_iterator it = jids.begin(); it != jids.end(); it++)
      {
         JobAf * job = NULL;
         if( *it == AFJOB::SYSJOB_ID )
            job = new SysJob( SysJob::FromDataBase);
         else
            job = new JobAf( *it);
         if( afDB_JobRegister.getItem( job))
         {
            if( *it == AFJOB::SYSJOB_ID )
            {
               SysJob * sysjob = (SysJob*)job;
               if( false == sysjob->isValid() )
               {
                  printf("System job retrieved from database is obsolete. Deleting it...\n");
                  std::list<std::string> queries;
                  job->dbDeleteNoStatistics( &queries);
                  delete job;
                  afDB_JobRegister.execute( &queries);
                  continue;
               }
               else
               {
                  printf("System job retrieved from database.\n");
                  hasSystemJob = true;
               }
            }
            jobs->job_register( job, users, NULL);
         }
         else
         {
            printf("Deleting invalid job from database...\n");
            std::list<std::string> queries;
            job->dbDeleteNoStatistics( &queries);
            job->stdOut();
            delete job;
            afDB_JobRegister.execute( &queries);
         }
      }
      printf("%d jobs founded.\n", (int)jids.size());

      //
      // Close database:
      //
      afDB_JobRegister.DBClose();
   }

//
// Create system maintenance job if it was not in database:
// (must be created after close of database connection to prevent mutex lock)
   if( hasSystemJob == false )
   {
      SysJob* job = new SysJob( SysJob::New);
      jobs->job_register( job, users, NULL);
   }

   initialized = true;
}

Core::~Core()
{

   if( core == NULL) return;
   core = NULL;

   AFINFO("Core::~Core: Destructor begin:")

   AFINFO("Core::~Core: Deleting read messages thread.")
   if( threadReadMsg    != NULL ) delete threadReadMsg;
   AFINFO("Core::~Core: Deleting run messages thread.")
   if( threadRun        != NULL ) delete threadRun;
   AFINFO("Core::~Core: Deleting messages queue.")
   if( msgQueue      != NULL ) delete msgQueue;
   AFINFO("Core::~Core: Deleting Talks container.")
   if( talks            != NULL ) delete talks;
   AFINFO("Core::~Core: Deleting Monitors container.")
   if( monitors         != NULL ) delete monitors;
   AFINFO("Core::~Core: Deleting Users container.")
   if( users            != NULL ) delete users;
   AFINFO("Core::~Core: Deleting Renders container.")
   if( renders          != NULL ) delete renders;
   AFINFO("Core::~Core: Deleting Jobs container.")
   if( jobs             != NULL ) delete jobs;

   AFINFO("Core::~Core: Destructed.")

}
