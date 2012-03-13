#include "theadrun.h"

#include <stdio.h>
#include <stdlib.h>

#include "../libafsql/qdbconnection.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

extern bool running;

TheadRun::TheadRun( const TreadPointers *ptrs, int seconds):
   TheadAf( ptrs)
{
   sec = seconds;
AFINFO("TheadRun::TheadRun:\n");
}

TheadRun::~TheadRun()
{
AFINFO("TheadRun::~TheadRun:\n");
}

void TheadRun::run()
{
AFINFO("TheadRun::run:\n");
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
AFINFO("TheadRun::run: Locking containers...\n");
      AfContainerLock jLock( jobs,     AfContainer::WRITELOCK);
      AfContainerLock lLock( renders,  AfContainer::WRITELOCK);
      AfContainerLock ulock( users,    AfContainer::WRITELOCK);
      AfContainerLock tlock( talks,    AfContainer::WRITELOCK);
      AfContainerLock mlock( monitors, AfContainer::WRITELOCK);

//
// Messages reaction:
//
AFINFO("TheadRun::run: React on incoming messages:\n");
      for( MsgAf *msg = msgQueue->popMsg( false); msg != NULL; msg = msgQueue->popMsg( false)) msgCase( msg);

//
// Refresh data:
//
AFINFO("TheadRun::run: Refreshing data:\n");
      talks    ->refresh( NULL,    monitors);
      monitors ->refresh( NULL,    monitors);
      jobs     ->refresh( renders, monitors);
      renders  ->refresh( jobs,    monitors);
      users    ->refresh( NULL,    monitors);

//
// Jobs sloving:
//
      AFINFO("TheadRun::run: Solving jobs:\n");
      RenderContainerIt rendersIt( renders);
      std::list<int> rIds;
      {
         // ask every ready render to produce a task
         for( RenderAf *render = rendersIt.render(); render != NULL; rendersIt.next(), render = rendersIt.render())
         {
            if( render->isReady())
            {
               // store render Id if it produced a task
               if( users->genTask( render, monitors)) rIds.push_back( render->getId());
            }
         }
      }
      // cycle on renders, which produced a task
      while( rIds.size())
      {
         AFINFA("TheadRun::run: Renders on cycle: %d\n", int(rIds.size()));
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
AFINFO("TheadRun::run: dispatching monitor events:\n");
      monitors->dispatch();

//
// Free Containers:
//
AFINFO("TheadRun::run: deleting zombies:\n");
      talks    ->freeZombies();
      monitors ->freeZombies();
      renders  ->freeZombies();
      jobs     ->freeZombies();
      users    ->freeZombies();

   }

//
// Sleeping
//
AFINFO("TheadRun::run: sleeping...\n");
   sleep( sec);
}
}
