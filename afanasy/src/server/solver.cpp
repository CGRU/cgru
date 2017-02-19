#include "solver.h"

#include "../include/afanasy.h"
#include "../libafanasy/environment.h"

#include "afnodesrv.h"
#include "jobcontainer.h"
#include "rendercontainer.h"
#include "usercontainer.h"
#include "monitorcontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

Solver::Solver(
		JobContainer     * i_jobcontainer,
		RenderContainer  * i_rendercontainer,
		UserContainer    * i_usercontainer,
		MonitorContainer * i_monitorcontainer
	):
	m_jobcontainer    ( i_jobcontainer     ),
	m_rendercontainer ( i_rendercontainer  ),
	m_usercontainer   ( i_usercontainer    ),
	m_monitorcontaier ( i_monitorcontainer )
{}

Solver::~Solver(){}

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

// Functor for sorting algorithm
struct GreaterNeed : public std::binary_function<AfNodeSrv*,AfNodeSrv*,bool>
{
    inline bool operator()(const AfNodeSrv * a, const AfNodeSrv * b)
    {
        return a->greaterNeed( b);
    }
};

// Other functor for an alternative sorting algorithm
struct GreaterPriorityThenOlderCreation : public std::binary_function<AfNodeSrv*,AfNodeSrv*,bool>
{
    inline bool operator()(const AfNodeSrv * a, const AfNodeSrv * b)
    {
        return a->greaterPriorityThenOlderCreation( b);
    }
};

void Solver::solve()
{
	//
	// Jobs sloving:
	//
	AF_DEBUG << "Solving jobs...";

	int tasks_solved = 0;

	std::list<AfNodeSrv*> solve_list;
	if( af::Environment::getSolvingUseUserPriority())
		solve_list = m_usercontainer->getNodesStdList();
	else
		solve_list = m_jobcontainer->getNodesStdList();

	std::list<RenderAf*> renders;
	std::list<RenderAf*> solved_renders;

	RenderContainerIt rendersIt( m_rendercontainer);
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
			bool solved = SolveList( solve_list, af::Node::SolveByPriority, render, m_monitorcontaier);

			if( solved)
			{
				// store render Id if it produced a task
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
				bool solved = SolveList( solve_list, af::Node::SolveByPriority, render, m_monitorcontaier);

				if( solved)
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

	//
	// Wake-On-Lan:
	//
	AF_DEBUG << "Wake-On-Lan...";

	rendersIt.reset();

	for( RenderAf *render = rendersIt.render(); render != NULL; rendersIt.next(), render = rendersIt.render())
	{
		if( render->isWOLWakeAble())
		{
			bool solved = SolveList( solve_list, af::Node::SolveByPriority, render, m_monitorcontaier);

			if( solved)
			{
				render->wolWake( m_monitorcontaier, std::string("Automatic waking by a job."));
				continue;
			}
		}
	}
}

/// Static function to solve nodes list:
bool Solver::SolveList( std::list<AfNodeSrv*> & i_list, af::Node::SolvingMethod i_method,
		RenderAf * i_render, MonitorContainer * i_monitoring)
{
	if( i_list.size() == 0 )
    {
        // No nodes - no solve needed
        return false;
    }

    std::list<AfNodeSrv*> solvelist;
    for( std::list<AfNodeSrv*>::const_iterator it = i_list.begin(); it != i_list.end(); it++)
	{
//printf("AfNodeSrv::solvelist: name = %s\n", (*it)->m_node->m_name.c_str());
        if((*it)->v_canRunOn( i_render))
       {
		   solvelist.push_back(*it);
       }
	}

    if( solvelist.size() == 0 )
    {
        // No nodes to solve
        return false;
    }

    // If not just do it by order, we should sort it:
    if( i_method != af::Node::SolveByOrder )
    {
        if (af::Environment::getSolvingSimpler())
            solvelist.sort( GreaterPriorityThenOlderCreation());
        else
            solvelist.sort( GreaterNeed());
        
    }

    // Try to solve most needed node
    for( std::list<AfNodeSrv*>::iterator it = solvelist.begin(); it != solvelist.end(); it++)
    {
        if((*it)->trySolve( i_render, i_monitoring))
        {
            // Return true - that some node was solved
           return true;
        }
    }

    // Return false - that no nodes was not solved
    return false;
}

