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
   afDB_JobRegister( "AFANASY_JobRegister"),
   init(false)
{
   core = this;
   AFINFO("Core::Core:\n");

   // containers initialization
   jobs = new JobContainer();
   if(  jobs == NULL){ AFERROR("Core::Core: Can't allocate memory for Jobs container."); return; }
   if( !jobs->isInitialized()) return;
   JobContainer::setDataBase( &afDB_JobRegister);
   JobAf::setJobContainer( jobs);

   users = new UserContainer();
   if(  users == NULL){ AFERROR("Core::Core: Can't allocate memory for Users container."); return; }
   if( !users->isInitialized()) return;

   renders = new RenderContainer();
   if(  renders == NULL){ AFERROR("Core::Core: Can't allocate memory for Renders container."); return; }
   if( !renders->isInitialized()) return;
   RenderAf::setRenderContainer( renders);

   talks = new TalkContainer();
   if( talks == NULL){ AFERROR("Core::Core: Can't allocate memory for Talks container."); return; }
   if( !talks->isInitialized()) return;

   monitors = new MonitorContainer();
   if( monitors == NULL){ AFERROR("Core::Core: Can't allocate memory for Monitors container."); return; }
   if( !monitors->isInitialized()) return;

   // check message queues initialization
   msgQueue = new MsgQueue("Run thread messages queue");
   if( msgQueue == NULL){ AFERROR("Core::Core: Can't allocate memory for run thread messages queue."); return; }
   if( !msgQueue->isInitialized()) return;

   // theads initialization
   ThreadPointers ptrs( this);
   threadRun = new ThreadRun( &ptrs);
   if( threadRun == NULL){ AFERROR("Core::Core: Can't allocate memory for run thread."); return; }
   threadReadMsg = new ThreadReadMsg( &ptrs);
   if( threadReadMsg == NULL){ AFERROR("Core::Core: Can't allocate memory for read messages thread."); return; }


//
// Open database to get nodes:
//
   afDB_JobRegister.DBOpen();
//
// Get Renders from database:
//
   AFINFO("Core::Core: Getting renders from database...\n")
   std::list<int> rids;
   afDB_JobRegister.getRendersIds( rids);
   for( std::list<int>::const_iterator it = rids.begin(); it != rids.end(); it++)
   {
      RenderAf * render = new RenderAf( *it);
      if( afDB_JobRegister.getItem( render)) renders->addRender( render);
      else delete render;
   }

//
// Get Users from database:
//
   AFINFO("Core::Core: Getting users from database...\n")
   std::list<int> uids;
   afDB_JobRegister.getUsersIds( uids);
   for( std::list<int>::const_iterator it = uids.begin(); it != uids.end(); it++)
   {
      UserAf * user = new UserAf( *it);
      if( afDB_JobRegister.getItem( user)) users->addUser( user);
      else delete user;
   }

//
// Get Jobs from database:
//
   AFINFO("Core::Core: Getting jobs from database...\n")
   bool hasSystemJob = false;
   std::list<int> jids;
   afDB_JobRegister.getJobsIds( jids);
   for( std::list<int>::const_iterator it = jids.begin(); it != jids.end(); it++)
   {
      JobAf * job = NULL;
      if( *it == AFJOB::SYSJOB_ID )
         job = new SysJob( SysJob::FromDataBase);
      else
         job = new JobAf( *it);
      if( afDB_JobRegister.getItem( job))
      {
         jobs->job_register( job, users, NULL);
         if( *it == AFJOB::SYSJOB_ID )
         {
            printf("System job retrieved from database.\n");
            hasSystemJob = true;
         }
      }
      else delete job;
   }

//
// Close database:
//
   afDB_JobRegister.DBClose();

//
// Create system maintenance job if it was not in database:
// (must be created after close of database connection to prevent mutex lock)
   if( hasSystemJob == false )
   {
      SysJob* job = new SysJob( SysJob::New);
      jobs->job_register( job, users, NULL);
   }

   init = true;
}

Core::~Core()
{

   if( core == NULL) return;
   core = NULL;

   AFINFO("Core::~Core: Destructor begin:\n");

   AFINFO("Core::~Core: Deleting read messages thread.\n");
   if( threadReadMsg    != NULL ) delete threadReadMsg;
   AFINFO("Core::~Core: Deleting run messages thread.\n");
   if( threadRun        != NULL ) delete threadRun;
   AFINFO("Core::~Core: Deleting messages queue.\n");
   if( msgQueue      != NULL ) delete msgQueue;
   AFINFO("Core::~Core: Deleting Talks container.\n");
   if( talks            != NULL ) delete talks;
   AFINFO("Core::~Core: Deleting Monitors container.\n");
   if( monitors         != NULL ) delete monitors;
   AFINFO("Core::~Core: Deleting Users container.\n");
   if( users            != NULL ) delete users;
   AFINFO("Core::~Core: Deleting Renders container.\n");
   if( renders          != NULL ) delete renders;
   AFINFO("Core::~Core: Deleting Jobs container.\n");
   if( jobs             != NULL ) delete jobs;

   AFINFO("Core::~Core: Destructed.\n");

}
