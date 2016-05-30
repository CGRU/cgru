#pragma once

//#include "../libafanasy/afnode.h"

#include "afnodesrv.h"
#include "afcontainer.h"

/// Afanasy nodes container locker.
/* Node is a base class of user, render and job */

class AfContainerLock
{
public:
	enum LockType
	{
		READLOCK,
		WRITELOCK
	};

public:
	AfContainerLock( AfContainer* afcontainer, LockType locktype);
	~AfContainerLock();

private:
	AfContainer* m_container;
	int m_type;
};

/// Afanasy container nodes iterator.
class AfContainerIt
{
public:

	/// Create an iterator for the container.
	AfContainerIt(AfContainer* af_container, bool skip_zombies = true);
	~AfContainerIt();
	
	void next();   ///< Set iterator position on next node.
	void reset();  ///< Set iterator position on the first node.
	
	inline AfNodeSrv * getNode() { return m_node; }
	
	/// Get node with specitied id. \c NULL returned if there is no node with such id.
	AfNodeSrv* get( int id);

private:
	AfNodeSrv * m_node;        ///< Current node pointer.
	AfContainer * m_container; ///< Container pointer.
	bool m_by_pass_zombies;    ///< Whether iterator will bypass zobmies in \c next() function.
};
