#include <stdio.h>
#include <stdlib.h>

#include "../libafanasy/environment.h"
#include "../libafanasy/msgqueue.h"

#include "auth.h"
#include "jobcontainer.h"
#include "monitorcontainer.h"
#include "rendercontainer.h"
#include "threadargs.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

extern bool AFRunning;

// Messages reaction case function
void threadRunCycleCase( ThreadArgs * i_args, af::Msg * i_msg);

struct MostReadyRender : public std::binary_function <RenderAf*,RenderAf*,bool>
{
	inline bool operator()( const RenderAf * a, const RenderAf * b)
	{
		if( a->getTasksNumber() < b->getTasksNumber()) return true;
		if( a->getTasksNumber() > b->getTasksNumber()) return false;

		if( a->getCapacityFree() > b->getCapacityFree()) return true;
		if( a->getCapacityFree() < b->getCapacityFree()) return false;

		if( a->getPriority() > b->getPriority()) return true;
		if( a->getPriority() < b->getPriority()) return false;

		if( a->getTasksStartFinishTime() < b->getTasksStartFinishTime()) return true;
		if( a->getTasksStartFinishTime() > b->getTasksStartFinishTime()) return false;

		if( a->getCapacity() > b->getCapacity()) return true;
		if( a->getCapacity() < b->getCapacity()) return false;

		if( a->getMaxTasks() > b->getMaxTasks()) return true;
		if( a->getMaxTasks() < b->getMaxTasks()) return false;

		return a->getName().compare( b->getName()) < 0;
	}
};

/** This is a main run cycle thread entry point
**/
void threadRunCycle( void * i_args)
{
	AFINFO("ThreadRun::run:")
	ThreadArgs * a = (ThreadArgs*)i_args;

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

	af::Msg *message;
	while( (message = a->msgQueue->popMsg( af::AfQueue::e_no_wait)) )
	{
		threadRunCycleCase( a, message );
	}

	//
	// Refresh data:
	//
	AFINFO("ThreadRun::run: Refreshing data:")
	a->monitors ->refresh( NULL,        a->monitors);
	a->jobs     ->refresh( a->renders,  a->monitors);
	a->renders  ->refresh( a->jobs,     a->monitors);
	a->users    ->refresh( NULL,        a->monitors);

	{
	//
	// Jobs sloving:
	//
	AFINFO("ThreadRun::run: Solving jobs:")

	int tasks_solved = 0;
	std::list<RenderAf*> renders;
	std::list<RenderAf*> solved_renders;

	RenderContainerIt rendersIt( a->renders);
	for( RenderAf *render = rendersIt.render(); render != NULL; rendersIt.next(), render = rendersIt.render())
		renders.push_back( render);

	renders.sort( MostReadyRender());

	// ask every ready render to produce a task
	for( std::list<RenderAf*>::iterator rIt = renders.begin(); rIt != renders.end(); rIt++)
	{
		if(( af::Environment::getServeTasksSpeed() >= 0 ) &&
			( tasks_solved >= af::Environment::getServeTasksSpeed()))
			break;

		RenderAf * render = *rIt;

		if( render->isReady())
		{
			// store render Id if it produced a task
			if( a->users->solve( render, a->monitors))
			{
				solved_renders.push_back( render);
				tasks_solved++;
				continue;
			}
		}

		// Render not solved, needed to update render status
		render->notSolved();
	}

	// cycle on renders, which produced a task
	static const int renders_cycle_limit = 100;
	int renders_cycle = 0;
	while( solved_renders.size())
	{
		renders_cycle++;
		if( renders_cycle > renders_cycle_limit )
		{
			AFERRAR("Renders solve cycles reached limit %d.", renders_cycle_limit)
			break;
		}

		if(( af::Environment::getServeTasksSpeed() >= 0 ) &&
			( tasks_solved >= af::Environment::getServeTasksSpeed()))
			break;

		solved_renders.sort( MostReadyRender());

		AFINFA("ThreadRun::run: Renders on cycle: %d", int(solved_renders.size()))
		std::list<RenderAf*>::iterator rIt = solved_renders.begin();
		while( rIt != solved_renders.end())
		{
			if(( af::Environment::getServeTasksSpeed() >= 0 ) &&
				( tasks_solved >= af::Environment::getServeTasksSpeed()))
				break;

			RenderAf * render = *rIt;
			if( render->isReady())
			{
				if( a->users->solve( render, a->monitors))
				{
					rIt++;
					tasks_solved++;
					continue;
				}
			}

			// delete render id from list if it can't produce a task
			rIt = solved_renders.erase( rIt);
		}
	}
	}// - jobs solving

	{
	//
	// Wake-On-Lan:
	//
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
	}// - wake-on-lan

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

	//
	// Sleeping
	//
	AFINFO("ThreadRun::run: sleeping...")
	   af::sleep_sec( 1);

	cycle++;
	}// - while running

}// - end of the thead function

