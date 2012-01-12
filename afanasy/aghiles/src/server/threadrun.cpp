#include "threadrun.h"

#include <stdio.h>
#include <stdlib.h>

#include "../libafsql/qdbconnection.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

extern bool running;

ThreadRun::ThreadRun( const ThreadPointers *ptrs, int seconds):
   ThreadAf( ptrs)
{
   sec = seconds;
AFINFO("ThreadRun::ThreadRun:")
}

ThreadRun::~ThreadRun()
{
AFINFO("ThreadRun::~ThreadRun:")
}

void ThreadRun::run()
{
AFINFO("ThreadRun::run:")
while( running)
{
#ifdef _DEBUG
printf("...................................\n");
#endif
/**/
   {
//
// Lock containers:
//
AFINFO("ThreadRun::run: Locking containers...")
      AfContainerLock jLock( jobs,     AfContainer::WRITELOCK);
      AfContainerLock lLock( renders,  AfContainer::WRITELOCK);
      AfContainerLock ulock( users,    AfContainer::WRITELOCK);
      AfContainerLock tlock( talks,    AfContainer::WRITELOCK);
      AfContainerLock mlock( monitors, AfContainer::WRITELOCK);

//
// Messages reaction:
//
AFINFO("ThreadRun::run: React on incoming messages:")

   /*
      Process all messages in our message queue. We do it without
      waiting so that the job solving below can run just after.
      NOTE: I think this should be a waiting operation in a different
      thread. The job solving below should be put asleep using a 
      semaphore and woke up when something changes. We need to avoid
      the Sleep() function below. 
   */

   MsgAf *message;
   while( message = msgQueue->popMsg(AfQueue::e_no_wait) )
   {
      msgCase( message );
   }

//
// Refresh data:
//
AFINFO("ThreadRun::run: Refreshing data:")
      talks    ->refresh( NULL,    monitors);
      monitors ->refresh( NULL,    monitors);
      jobs     ->refresh( renders, monitors);
      renders  ->refresh( jobs,    monitors);
      users    ->refresh( NULL,    monitors);

//
// Jobs sloving:
//
      AFINFO("ThreadRun::run: Solving jobs:")
      RenderContainerIt rendersIt( renders);
      std::list<int> rIds;
      {
         // ask every ready render to produce a task
         for( RenderAf *render = rendersIt.render(); render != NULL; rendersIt.next(), render = rendersIt.render())
         {
            if( render->isReady())
            {
               // store render Id if it produced a task
               if( users->genTask( render, monitors))
               {
                  rIds.push_back( render->getId());
                  continue;
               }
            }
            // Render not solved, needed to update render status
            render->notSolved();
         }
      }

      // cycle on renders, which produced a task
      while( rIds.size())
      {
         AFINFA("ThreadRun::run: Renders on cycle: %d", int(rIds.size()))
         std::list<int>::iterator rIt = rIds.begin();
         while( rIt != rIds.end())
         {
            RenderAf * render = rendersIt.getRender( *rIt);
            if( render->isReady())
            {
               if( users->genTask( render, monitors))
               {
                  rIt++;
                  continue;
               }
            }
            // delete render id from list if it can't produce a task
            rIt = rIds.erase( rIt);
         }
      }

//
// Dispatch events to monitors:
//
AFINFO("ThreadRun::run: dispatching monitor events:")
      monitors->dispatch();

//
// Free Containers:
//
AFINFO("ThreadRun::run: deleting zombies:")
      talks    ->freeZombies();
      monitors ->freeZombies();
      renders  ->freeZombies();
      jobs     ->freeZombies();
      users    ->freeZombies();

   }

//
// Sleeping
//
AFINFO("ThreadRun::run: sleeping...")
   sleep( sec);
}
}
