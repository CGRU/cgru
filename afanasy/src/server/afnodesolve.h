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
	afnodesolve.h - TODO: description
*/

#pragma once

#include "../libafanasy/afwork.h"

#include "afnodesrv.h"

class BranchSrv;

class AfNodeSolve : public AfNodeSrv
{
  public:
	AfNodeSolve(af::Work *i_work, const std::string &i_type_name, const std::string &i_store_dir);
	virtual ~AfNodeSolve();

	// Just interesting - good to show server load
	static unsigned long long getSolvesCount() { return sm_solve_cycle; }

	void v_preSolve(time_t i_curtime, MonitorContainer *i_monitors);

	/// Can node run
	/** Needed to limit nodes quantinity for solving algorithm, which can be heavy. **/
	virtual bool canRun();
	/// Virtual function to tune for each node type:
	virtual bool v_canRun();

	/// Can node run on specified render
	/** Needed to limit nodes quantinity for solving algorithm, which can be heavy. **/
	bool canRunOn(RenderAf *i_render);
	/// Virtual function to tune for each node type:
	virtual bool v_canRunOn(RenderAf *i_render);

	int getPoolPriority(const RenderAf *i_render) const;

	/// Calc node need for solving.
	/// i_flags - solving flags (to use running_capacity_total or running_tasks_num)
	/// i_resourcesquantity - to use specified resources quantinity (if != -1), i_flags will be ignored
	void calcNeed(int i_flags, int i_resourcesquantity = -1);

	// Try to solve a node, v_solve is called there:
	RenderAf *solve(std::list<RenderAf *> &i_renders_list, MonitorContainer *i_monitoring,
					BranchSrv *i_branch);

	/// Solving function should be implemented in child classes (if solving needed):
	/// Generate task for \c some render from list, return \c render if task generated or NULL.
	virtual RenderAf *v_solve(std::list<RenderAf *> &i_renders_list, MonitorContainer *i_monitoring,
							  BranchSrv *i_branch);

	/// Compare nodes solving need:
	bool greaterNeed(const AfNodeSolve *i_other) const;
	bool greaterPriorityThenOlderCreation(const AfNodeSolve *i_other) const;

	void addSolveCounts(af::TaskExec *i_exec, RenderAf *i_render);
	void remSolveCounts(af::TaskExec *i_exec, RenderAf *i_render);

	void addRendersCounts(const AfNodeSolve &i_other);
	void remRendersCounts(const AfNodeSolve &i_other);

	void setZombie();

	friend class AfList;

  private:
	/// Renders counts manipulations (for max run tasks per host)
	void addRenderCount(int i_render_id, int i_count = 1);
	void remRenderCount(int i_render_id, int i_count = 1);
	int getRenderCount(RenderAf *i_render) const;

  private:
	af::Work *m_work;

	/// List of lists which have this node ( for a exapmle: each user has some jobs).
	std::list<AfList *> m_lists;

	/// Renders counts for max run tasks per host:
	std::map<int, int> m_renders_counts;

	/// Will be incremented on each solve on any node
	/** 2^64 / ( seconds_in_year * million_solves_persecond ) ~ 600 thousands of years to work with no
	 *overflow
	 *** million solves per second is unreachable parameter **/
	static unsigned long long sm_solve_cycle;
	/*  If you are able to produce and solve 117 tasks per second all day long and every day,
		you have a chance to overflow it before the sun will finish to shine in 5 billions years */

	/// A node with maximum need value will take next free host.
	float m_solve_need;

	/// Last solved cycle.
	/** Needed to jobs (users) solving, to compare nodes solving order.**/
	unsigned long long m_solve_cycle;

	/// Number of time node was sucessfuly solved on a last run cycle.
	int m_solves_count;
};
