#pragma once

#include "../libafanasy/afnode.h"

class Action;

class AfNodeSrv
{
public:
	AfNodeSrv( af::Node * i_node, const std::string & i_store_dir = "");
	virtual ~AfNodeSrv();

	inline af::Node * node() { return m_node; }

	void store() const;

	inline bool isFromStore() const { return m_from_store; }
	inline const std::string & getStoreDir() const { return m_store_dir; }
	inline const std::string & getStoreFile() const { return m_store_file; }
	bool createStoreDir() const;

	/// Nodes comparison by priority ( wich is private property).
	inline bool operator <  ( const AfNodeSrv & other) const { return m_node->m_priority <  other.m_node->m_priority;}
	inline bool operator <= ( const AfNodeSrv & other) const { return m_node->m_priority <= other.m_node->m_priority;}
	inline bool operator >  ( const AfNodeSrv & other) const { return m_node->m_priority >  other.m_node->m_priority;}
	inline bool operator >= ( const AfNodeSrv & other) const { return m_node->m_priority >= other.m_node->m_priority;}
	inline bool operator == ( const AfNodeSrv & other) const { return m_node->m_priority == other.m_node->m_priority;}
	inline bool operator != ( const AfNodeSrv & other) const { return m_node->m_priority != other.m_node->m_priority;}

	/// Set some node attribute by incoming message.
	void action( Action & i_action);

	virtual void v_action( Action & i_action);

	/// Refresh node information
	virtual void v_refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);

	// Friends for container:
	friend class AfContainer;
	friend class AfContainerIt;
	friend class AfList;
	friend class AfListIt;

	inline void lock()     const { m_node->m_locked =  true; }
	inline void unLock()   const { m_node->m_locked = false; }

	virtual void v_setZombie() { m_node->m_flags = m_node->m_flags | af::Node::FZombie; } ///< Request to kill a node.

	void appendLog( const std::string & message);  ///< Append task log with a \c message .
	inline const std::list<std::string> & getLog() { return m_log; }    ///< Get log.
	af::Msg * writeLog() const;
	int calcLogWeight() const;

	// Just interesting - good to show server load
	static unsigned long long getSolvesCount() { return sm_solve_cycle; }

	/// Can node run
	/** Needed to limit nodes quantinity for solving algorithm, which can be heavy. **/
	virtual bool v_canRun();

	/// Can node run on specified render
	/** Needed to limit nodes quantinity for solving algorithm, which can be heavy. **/
	virtual bool v_canRunOn( RenderAf * i_render);

	/// Solve nodes list:
	static bool solveList( std::list<AfNodeSrv*> & i_list, af::Node::SolvingMethod i_method, RenderAf * i_render,
				MonitorContainer * i_monitoring);

	/// Main solving function should be implemented in child classes (if solving needed):
	virtual bool v_solve( RenderAf * i_render, MonitorContainer * i_monitoring);

	/// Compare nodes solving need:
	bool greaterNeed( const AfNodeSrv * i_other) const;

protected:
	void setStoreDir( const std::string & i_store_dir);

	/// General need calculation function,
	/** Some resources should be passed to its algorithm.**/
	void calcNeedResouces( int i_resourcesquantity);

	/// Virtual function to calculate need.
	/** Node should define what resource shoud be passed for need calculation.**/
	virtual void v_calcNeed();

private:
	// Try to solve a node
	bool trySolve( RenderAf * i_render, MonitorContainer * i_monitoring);

	virtual void v_priorityChanged( MonitorContainer * i_monitoring);

private:
//public:

	af::Node * m_node;
	bool m_from_store;            ///< Whether the node constructed from store.
	std::string m_store_dir;   ///< Store directory.
	std::string m_store_file;  ///< Store file.

private:
/// When node is ready to be deleted from container its becames a zombie and wait for a deletion by special thread.
//   bool zombie;

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

/// Previous node pointer. Previous container node has a greater or equal priority.
	AfNodeSrv * m_prev_ptr;

/// Next node pointer. Next container node has a less or equal priority.
	AfNodeSrv * m_next_ptr;

/// List of lists which have this node ( for a exapmle: each user has some jobs).
	std::list<AfList*> m_lists;

	std::list<std::string> m_log;                          ///< Log.
};
