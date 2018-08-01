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
	afnodesolve.cpp - TODO: description
*/

#include "afnodesolve.h"

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"

#include "aflist.h"
#include "renderaf.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

// Zero solve cycle variable in nodes is initial,
// it means that node was not solved at all.
unsigned long long AfNodeSolve::sm_solve_cycle = 1;

AfNodeSolve::AfNodeSolve(af::Work *i_work, const std::string &i_store_dir)
	: AfNodeSrv(i_work, i_store_dir),
	  m_work(i_work),
	  m_solve_need(0.0),
	  m_solve_cycle(0), // 0 means that it was not solved at all
	  m_solves_count(0)
{
}

AfNodeSolve::~AfNodeSolve()
{
}

void AfNodeSolve::setZombie()
{
	std::list<AfList *>::iterator it = m_lists.begin();
	std::list<AfList *>::iterator end_it = m_lists.end();
	while (it != end_it)
		(*it++)->remove(this);

	AfNodeSrv::setZombie();
}

void AfNodeSolve::addSolveCounts(af::TaskExec *i_exec, RenderAf *i_render)
{
	m_work->addRunTasksCounts(i_exec);
	addRenderCount(i_render->getId());
}

void AfNodeSolve::remSolveCounts(af::TaskExec *i_exec, RenderAf *i_render)
{
	m_work->remRunTasksCounts(i_exec);
	remRenderCount(i_render->getId());
}

void AfNodeSolve::addRenderCount(int i_render_id, int i_count)
{
	// Add render count, on task start:
	std::map<int, int>::iterator it = m_renders_counts.find(i_render_id);
	if (it != m_renders_counts.end())
		(*it).second += i_count;
	else
		m_renders_counts[i_render_id] = i_count;
}

int AfNodeSolve::getRenderCount(RenderAf *i_render) const
{
	// Get render count, to check max run tasks per host limit:
	std::map<int, int>::const_iterator it = m_renders_counts.find(i_render->getId());
	if (it != m_renders_counts.end()) return (*it).second;
	return 0;
}

void AfNodeSolve::remRenderCount(int i_render_id, int i_count)
{
	// Remove one render count, on task finish:
	std::map<int, int>::iterator it = m_renders_counts.find(i_render_id);
	if (it != m_renders_counts.end())
	{
		(*it).second -= i_count;

		if ((*it).second < 0)
		{
			AF_ERR << "AfNodeSolve::remRenderCount[" << m_node->getName() << "]: Render[" << i_render_id << "] count is negative: " << (*it).second;
			(*it).second = 0;
		}

		if ((*it).second == 0)
			m_renders_counts.erase(it);
	}
}

void AfNodeSolve::addRendersCounts(const AfNodeSolve & i_other)
{
	std::map<int, int>::const_iterator it = i_other.m_renders_counts.begin();
	for (; it != i_other.m_renders_counts.end(); it++)
		addRenderCount((*it).first, (*it).second);
}

void AfNodeSolve::remRendersCounts(const AfNodeSolve & i_other)
{
	std::map<int, int>::const_iterator it = i_other.m_renders_counts.begin();
	for (; it != i_other.m_renders_counts.end(); it++)
		remRenderCount((*it).first, (*it).second);
}

void AfNodeSolve::v_preSolve(time_t i_curtime, MonitorContainer * i_monitors)
{
	// Resets solves count to max_tasks_per_second limit
	m_solves_count = 0;
}

/// Solving function should be implemented in child classes (if solving needed):
RenderAf *AfNodeSolve::v_solve(std::list<RenderAf *> &i_renders_list, MonitorContainer *i_monitoring, BranchSrv * i_branch)
{
	AF_ERR << "AfNodeSrv::solve(): Not implemented: " << m_node->getName();
	return NULL;
}
void AfNodeSolve::calcNeed(int i_flags, int i_resourcesquantity)
{
	int resourcesquantity = i_resourcesquantity;

	if (resourcesquantity < 0)
	{
		if (i_flags & af::Work::SolveCapacity)
			resourcesquantity = m_work->getRunningCapacityTotal();
		else
			resourcesquantity = m_work->getRunningTasksNum();
	}


	// Less resources less need
	m_solve_need = pow(1.1, m_node->getPriority()) / (resourcesquantity + 1.0);
	/// each priority point gives 10% more resources
}

bool AfNodeSolve::canRun()
{
	if (m_work->getMaxTasksPerSecond() == 0)
	{
		// Solving is off totally for this node
		return false;
	}

	if ((m_work->getMaxTasksPerSecond() > 0) && (m_solves_count >= m_work->getMaxTasksPerSecond()))
	{
		// Solving speed (tasks per second) reached the limit
		return false;
	}

	if (m_work->getPriority() == 0)
	{
		// Zero priority turns solving off
		return false;
	}

	if (m_work->getMaxRunningTasks() == 0)
	{
		// Can't have running tasks at all, turns solving off
		return false;
	}

	if (m_work->getMaxRunTasksPerHost() == 0)
	{
		// Can't run tasks on any host at all, turns solving off
		return false;
	}

	// Check maximum running tasks:
	if ((m_work->getMaxRunningTasks() > 0) && (m_work->getRunningTasksNum() >= m_work->getMaxRunningTasks()))
	{
		return false;
	}

	// Perform each node type scpecific check
	return v_canRun();
}
bool AfNodeSolve::v_canRun()
{
	AF_ERR << "AfNodeSolve::canRun(): Not implememted: " << m_node->getName().c_str();
	return false;
}

bool AfNodeSolve::canRunOn(RenderAf *i_render)
{
	// Check maximum running tasks per host
	if ((m_work->getMaxRunTasksPerHost() > 0)
		&& (getRenderCount(i_render) >= m_work->getMaxRunTasksPerHost()))
	{
		return false;
	}

	// Check hosts mask
	if (false == m_work->checkHostsMask(i_render->getName()))
	{
		return false;
	}

	// Check exclude hosts mask
	if (false == m_work->checkHostsMaskExclude(i_render->getName()))
	{
		return false;
	}

	// Perform each node type scpecific check
	return v_canRunOn(i_render);
}
bool AfNodeSolve::v_canRunOn(RenderAf *i_render)
{
	AF_ERR << "AfNodeSolve::canRunOn(): Not implememted: " << m_node->getName().c_str();
	return false;
}

/// Compare nodes need for solve:
bool AfNodeSolve::greaterNeed(const AfNodeSolve *i_other) const
{
	if (m_solve_need > i_other->m_solve_need) return true;

	if (m_solve_need < i_other->m_solve_need) return false;

	/// If need parameters are equal,
	/// Greater node is a node that was solved earlier
	return m_solve_cycle < i_other->m_solve_cycle;
}

bool AfNodeSolve::greaterPriorityThenOlderCreation(const AfNodeSolve *i_other) const
{
	if (m_node->getPriority() != i_other->m_node->getPriority())
		return m_node->getPriority() > i_other->m_node->getPriority();

	// If the priority is the same, we look for the smaller creation time
	if (m_node->getTimeCreation() != i_other->m_node->getTimeCreation())
		return m_node->getTimeCreation() < i_other->m_node->getTimeCreation();

	// If creation time is the same too (likely because this type of node does not implement getTimeCreation),
	// use the earliest solved node
	return m_solve_cycle < i_other->m_solve_cycle;
}

RenderAf *AfNodeSolve::solve(std::list<RenderAf *> &i_renders_list, MonitorContainer *i_monitoring, BranchSrv * i_branch)
{
	RenderAf *render = v_solve(i_renders_list, i_monitoring, i_branch);

	if (NULL == render)
	{
		// Was not solved
		return NULL;
	}

	// Node solved successfully:

	// Store solve cycle
	m_solve_cycle = sm_solve_cycle;

	// Icrement a global (static) solve cycle
	sm_solve_cycle++;

	// Increment solves count (local, per run cycle)
	m_solves_count++;

	// Returning that node was solved
	return render;
}
