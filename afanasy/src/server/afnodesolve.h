#pragma once

#include "../libafanasy/afwork.h"

#include "afnodesrv.h"

class AfNodeSolve : public AfNodeSrv
{
public:
	AfNodeSolve( af::Work * i_work, const std::string & i_store_dir = "");
	virtual ~AfNodeSolve();

	// Just interesting - good to show server load
	static unsigned long long getSolvesCount() { return sm_solve_cycle; }

	/// Can node run
	/** Needed to limit nodes quantinity for solving algorithm, which can be heavy. **/
	virtual bool canRun();
	/// Virtual function to tune for each node type:
	virtual bool v_canRun();

	/// Can node run on specified render
	/** Needed to limit nodes quantinity for solving algorithm, which can be heavy. **/
	bool canRunOn(RenderAf * i_render);
	/// Virtual function to tune for each node type:
	virtual bool v_canRunOn(RenderAf * i_render);

	/// Virtual function to calculate need.
	/** Node should define what resource shoud be passed for need calculation.**/
	virtual void v_calcNeed();

	// Try to solve a node, v_solve is called there:
	RenderAf * trySolve( std::list<RenderAf*> & i_renders_list, MonitorContainer * i_monitoring);

	/// Solving function should be implemented in child classes (if solving needed):
	/// Generate task for \c some render from list, return \c render if task generated or NULL.
	virtual RenderAf * v_solve( std::list<RenderAf*> & i_renders_list, MonitorContainer * i_monitoring);

	/// Compare nodes solving need:
	bool greaterNeed( const AfNodeSolve * i_other) const;
	bool greaterPriorityThenOlderCreation( const AfNodeSolve * i_other) const;

	void addSolveCounts(af::TaskExec * i_exec, RenderAf * i_render);
	void remSolveCounts(af::TaskExec * i_exec, RenderAf * i_render);

	void setZombie();

	friend class AfList;

protected:
	/// General need calculation function,
	/** Some resources should be passed to its algorithm.**/
	void calcNeedResouces( int i_resourcesquantity);

private:
	/// Renders counts manipulations (for max run tasks per host)
	void addRenderCount(RenderAf * i_render);
	int  getRenderCount(RenderAf * i_render) const;
	void remRenderCount(RenderAf * i_render);

private:
	af::Work * m_work;

/// List of lists which have this node ( for a exapmle: each user has some jobs).
	std::list<AfList*> m_lists;

/// Renders counts for max run tasks per host:
	std::map<int,int> m_renders_counts;

/// Will be incremented on each solve on any node
/** 2^64 / ( seconds_in_year * million_solves_persecond ) ~ 600 thousands of years to work with no overflow
*** million solves per second is unreachable parameter **/
	static unsigned long long sm_solve_cycle;
/*  If you are able to produce and solve 117 tasks per second all day long and every day,
    you have a chance to overflow it before the sun will finish to shine in 5 billions years */

/// A node with maximum need value will take next free host.
	float m_solve_need;

/// Last solved cycle.
/** Needed to jobs (users) solving, to compare nodes solving order.**/
	unsigned long long m_solve_cycle;
};
