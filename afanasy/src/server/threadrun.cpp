#include <stdio.h>
#include <stdlib.h>

#include "../libafanasy/msgqueue.h"

#include "jobcontainer.h"
#include "monitorcontainer.h"
#include "rendercontainer.h"
#include "talkcontainer.h"
#include "threadargs.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

extern bool AFRunning;

// Messages reaction case function
void threadRunCycleCase( ThreadArgs * i_args, af::Msg * i_msg);

/** This is a main run cycle thread entry point
**/
void threadRunCycle( void * i_args)
{
   AFINFO("ThreadRun::run:")
   ThreadArgs * a = (ThreadArgs*)i_args;

while( AFRunning)
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
      AfContainerLock jLock( a->jobs,     AfContainerLock::WRITELOCK);
      AfContainerLock lLock( a->renders,  AfContainerLock::WRITELOCK);
      AfContainerLock mlock( a->monitors, AfContainerLock::WRITELOCK);
      AfContainerLock tlock( a->talks,    AfContainerLock::WRITELOCK);
      AfContainerLock ulock( a->users,    AfContainerLock::WRITELOCK);

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

   af::Msg *message;
   while( message = a->msgQueue->popMsg( af::AfQueue::e_no_wait) )
   {
      threadRunCycleCase( a, message );
   }

//
// Refresh data:
//
AFINFO("ThreadRun::run: Refreshing data:")
      a->talks    ->refresh( NULL,        a->monitors);
      a->monitors ->refresh( NULL,        a->monitors);
      a->jobs     ->refresh( a->renders,  a->monitors);
      a->renders  ->refresh( a->jobs,     a->monitors);
      a->users    ->refresh( NULL,        a->monitors);

//
// Jobs sloving:
//
		{
      AFINFO("ThreadRun::run: Solving jobs:")
      RenderContainerIt rendersIt( a->renders);
      std::list<int> rIds;
      {
         // ask every ready render to produce a task
         for( RenderAf *render = rendersIt.render(); render != NULL; rendersIt.next(), render = rendersIt.render())
         {
            if( render->isReady())
            {
               // store render Id if it produced a task
               if( a->users->solve( render, a->monitors))
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
		static const int renders_cycle_limit = 100000;
		int renders_cycle = 0;
		while( rIds.size())
		{
			renders_cycle++;
			if( renders_cycle > renders_cycle_limit )
			{
				AFERROR("Renders solve cycles limit reached.");
				break;
			}
         AFINFA("ThreadRun::run: Renders on cycle: %d", int(rIds.size()))
         std::list<int>::iterator rIt = rIds.begin();
         while( rIt != rIds.end())
         {
            RenderAf * render = rendersIt.getRender( *rIt);
            if( render->isReady())
            {
               if( a->users->solve( render, a->monitors))
               {
                  rIt++;
                  continue;
               }
            }
            // delete render id from list if it can't produce a task
            rIt = rIds.erase( rIt);
         }
      }
		}

//
// Wake-On-Lan:
//
		{
		AFINFO("ThreadRun::run: Wake-On-Lan:")
		RenderContainerIt rendersIt( a->renders);
		{
			for( RenderAf *render = rendersIt.render(); render != NULL; rendersIt.next(), render = rendersIt.render())
			{
				if( render->isWOLWakeAble())
				{
					if( a->users->solve( render, a->monitors))
					{
						render->wolWake( a->monitors, std::string("Automatic waking by a job."));
						continue;
					}
				}
			}
		}
		}

//
// Dispatch events to monitors:
//
AFINFO("ThreadRun::run: dispatching monitor events:")
      a->monitors->dispatch();

//
// Free Containers:
//
AFINFO("ThreadRun::run: deleting zombies:")
      a->talks    ->freeZombies();
      a->monitors ->freeZombies();
      a->renders  ->freeZombies();
      a->jobs     ->freeZombies();
      a->users    ->freeZombies();

   }

//
// Sleeping
//
AFINFO("ThreadRun::run: sleeping...")
   af::sleep_sec( 1);
}
}
