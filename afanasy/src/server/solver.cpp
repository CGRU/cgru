/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

/*
	Solver class.
	Designed to encapsulate functions to solve jobs on renders.
*/
#include "solver.h"

#include "../include/afanasy.h"
#include "../libafanasy/environment.h"

#include "afnodesolve.h"
#include "branchescontainer.h"
#include "jobcontainer.h"
#include "rendercontainer.h"
#include "usercontainer.h"
#include "monitorcontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

BranchesContainer * Solver::ms_branchescontainer = NULL;
JobContainer      * Solver::ms_jobcontainer      = NULL;
RenderContainer   * Solver::ms_rendercontainer   = NULL;
UserContainer     * Solver::ms_usercontainer     = NULL;
MonitorContainer  * Solver::ms_monitorcontaier   = NULL;

uint64_t Solver::ms_run_cycle = 0;
const int Solver::ms_solve_cycles_limit = 11000;
int Solver::ms_awaken_renders = 0;
const int Solver::ms_awaken_renders_max = 1;

Solver::Solver(
		BranchesContainer * i_branchescontainer,
		JobContainer      * i_jobcontainer,
		RenderContainer   * i_rendercontainer,
		UserContainer     * i_usercontainer,
		MonitorContainer  * i_monitorcontainer
	)
{
	ms_branchescontainer = i_branchescontainer;
	ms_jobcontainer      = i_jobcontainer;
	ms_rendercontainer   = i_rendercontainer;
	ms_usercontainer     = i_usercontainer;
	ms_monitorcontaier   = i_monitorcontainer;
}

Solver::~Solver(){}

class MostReadyRender : public std::binary_function <RenderAf*,RenderAf*,bool>
{
	public:
	inline bool operator()( const RenderAf * a, const RenderAf * b)
	{
		// Offline renders needed for Wake-On-Lan.
		// Offline render is less ready.
		if( a->isOnline() && b->isOffline()) return true;
		if( a->isOffline() && b->isOnline()) return false;

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
struct GreaterNeed : public std::binary_function<AfNodeSolve*,AfNodeSolve*,bool>
{
	inline bool operator()(const AfNodeSolve * a, const AfNodeSolve * b)
	{
		return a->greaterNeed( b);
	}
};

// Other functor for an alternative sorting algorithm
struct GreaterPriorityThenOlderCreation : public std::binary_function<AfNodeSolve*,AfNodeSolve*,bool>
{
	inline bool operator()(const AfNodeSolve * a, const AfNodeSolve * b)
	{
		return a->greaterPriorityThenOlderCreation( b);
	}
};

void Solver::SortList(std::list<AfNodeSolve*> & i_list, int i_solving_flags)
{
	if (i_solving_flags & af::Work::SolvePriority)
		i_list.sort(GreaterNeed());
	else
		i_list.sort(GreaterPriorityThenOlderCreation());
}

void Solver::solve()
{
	//
	// Jobs solving:
	//
	ms_run_cycle++;
	AF_DEBUG << "Solving jobs...";

	// To start solving we need to solve the root branch:
	std::list<AfNodeSolve*> solve_list;
	solve_list.push_back(ms_branchescontainer->getRootBranch());

//########################################

	int solve_cycle = 0;
	int tasks_solved = 0;
	ms_awaken_renders = 0;

	// Start solve cycle.
	// If some node was solved it means that it can be solved again.
	// If some node was not solved it can't be solved again (before something changed),
	// and not solved node will be removed from list.
	while (solve_list.size())
	{
		// Increment cycle and check limit:
		solve_cycle++;
		if (solve_cycle > ms_solve_cycles_limit)
		{
			// This should not happen.
			// Most probably it is a bug in solving code.
			AF_WARN << "Solve cycles limit reached: " << ms_solve_cycles_limit;
			break;
		}

		// Check that the root branch can run,
		// may be solving is off by some limit for maintenance.
		if (false == ms_branchescontainer->getRootBranch()->canRun())
			break;

		// Get ready renders:
		std::list<RenderAf*> renders_list;
		RenderContainerIt rendersIt(ms_rendercontainer);
		for (RenderAf * render = rendersIt.render(); render != NULL; rendersIt.next(), render = rendersIt.render())
		{
			// Check that render is ready to run a task:
			if (false == render->isReady())
			{
				// Render is not ready, but may be we should wake it up?
				if ((false == render->isWOLWakeAble()) ||
					(ms_awaken_renders >= ms_awaken_renders_max) ||
					(ms_run_cycle % af::Environment::getWOLWakeInterval() != 0))
				{
					continue; ///< - We should not.
				}
			}

			renders_list.push_back(render);
		}

		// Function exits on each solve success (just 1 task solved),
		// removes nodes that was not solved from list.
		RenderAf * render = SolveList(solve_list, renders_list, ms_branchescontainer->getRootBranch());
		if (render)
		{
			// Check Wake-On-LAN:
			if (render->isWOLWakeAble())
			{
				AF_DEBUG << "Solving waking up render '" << render->node()->getName() << "'.";
				render->wolWake(ms_monitorcontaier, std::string("Automatic waking by a job."));
				ms_awaken_renders++;
			}
			else
				tasks_solved++;
		}
	}

	AF_DEBUG << "Solved " << tasks_solved << " tasks within " << solve_cycle << " cycles.";
}

RenderAf * Solver::SolveList(std::list<AfNodeSolve*> & i_list, std::list<RenderAf*> & i_renders, BranchSrv * i_branch)
{
	// Iterate solving nodes list:
	for (std::list<AfNodeSolve*>::iterator it = i_list.begin(); it != i_list.end(); )
	{
		// Get renders that node can run on:
		std::list<RenderAf*> renders;
		for (std::list<RenderAf*>::iterator rIt = i_renders.begin(); rIt != i_renders.end(); rIt++)
		{
			// Check that the node can run this render:
			if (false == (*it)->canRunOn(*rIt))
				continue;

			renders.push_back(*rIt);
		}

		// Sort renders:
		renders.sort(MostReadyRender());

		RenderAf * render = (*it)->solve(renders, ms_monitorcontaier, i_branch);

		if (render)
			return render;

		it = i_list.erase(it);
	}

	return NULL;
}

