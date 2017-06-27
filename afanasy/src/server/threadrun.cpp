#include <stdio.h>
#include <stdlib.h>

#include "../libafanasy/environment.h"
#include "../libafanasy/msgqueue.h"

#include "afcommon.h"
#include "auth.h"
#include "jobcontainer.h"
#include "monitorcontainer.h"
#include "rendercontainer.h"
#include "socketsprocessing.h"
#include "solver.h"
#include "threadargs.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

#include "../libafanasy/logger.h"

extern bool AFRunning;

// Messages reaction case function
void threadRunCycleCase( ThreadArgs * i_args, af::Msg * i_msg);

/** This is a main run cycle thread entry point
**/
void threadRunCycle( void * i_args)
{
	AF_LOG << "Run thread started.";

	ThreadArgs * a = (ThreadArgs*)i_args;

	// Jobs solving class:
	Solver solver( a->jobs, a->renders, a->users, a->monitors);

	// Save store to store start time:
	AFCommon::saveStore();

	long long cycle = 0;

	while( AFRunning)
	{
	#ifdef _DEBUG
	printf("...................................\n");
	#endif

	//
	// Free authentication clients store:
	//
	//if( cycle % 10 == 0 )
		Auth::free();

	{
	//
	// Lock containers:
	//
	AFINFO("ThreadRun::run: Locking containers...")
	AfContainerLock jLock( a->jobs,     AfContainerLock::WRITELOCK);
	AfContainerLock lLock( a->renders,  AfContainerLock::WRITELOCK);
	AfContainerLock mlock( a->monitors, AfContainerLock::WRITELOCK);
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

	//
	// Update tasks from render updates:
	//
	af::RenderUpdate * rup;
	while( (rup = a->rupQueue->popUp( af::AfQueue::e_no_wait)) )
	{
		for( int i = 0; i < rup->m_taskups.size(); i++)
			a->jobs->updateTaskState( *(rup->m_taskups[i]), a->renders, a->monitors);

		delete rup;
	}

	//
	// React on incomming connections:
	//
	a->socketsProcessing->processRun();

	//
	// Refresh data:
	//
	AFINFO("ThreadRun::run: Refreshing data:")
	a->monitors ->refresh( NULL,        a->monitors);
	a->jobs     ->refresh( a->renders,  a->monitors);
	a->renders  ->refresh( a->jobs,     a->monitors);
	a->users    ->refresh( NULL,        a->monitors);


	//
	// Jobs sloving:
	//
	solver.solve();
	
	//
	// Dispatch events to monitors:
	//
	AFINFO("ThreadRun::run: dispatching monitor events:")
	a->monitors->dispatch( a->renders);

	//
	// Free Containers:
	//
	AFINFO("ThreadRun::run: deleting zombies:")
	a->monitors ->freeZombies();
	a->renders  ->freeZombies();
	a->jobs     ->freeZombies();
	a->users    ->freeZombies();

	}// - lock containers

	// Save store
	if( cycle % 100 == 0 )
	{
		// Store should be save on change.
		// But it can be also used to see some statistics.
		AFCommon::saveStore();
	}

	//
	// Sleeping
	//
	AFINFO("ThreadRun::run: sleeping...")
	   af::sleep_sec( 1);

	cycle++;
	}// - while running

	// Save store on exit:
	AFCommon::saveStore();
	AF_LOG << "Run thread finished.";

}// - end of the thead function

