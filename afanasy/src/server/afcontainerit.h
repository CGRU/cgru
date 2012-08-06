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
   AfContainer* container;
   int type;
};

/// Afanasy container nodes iterator.
class AfContainerIt
{
public:

	/// Create an iterator for the container.
	AfContainerIt( AfContainer* afContainer, bool skipZombies = true);
   ~AfContainerIt();

   void next();   ///< Set iterator position on next node.
   void reset();  ///< Set iterator position on the first node.

	inline AfNodeSrv * getNode() { return node; }

	/// Get node with specitied id. \c NULL returned if there is no node with such id.
	AfNodeSrv* get( int id);

private:
	AfNodeSrv * node; ///< Current node pointer.
	AfContainer * container; ///< Container pointer.
	bool byPassZombies;     ///< Whether iterator will bypass zobmies in \c next() function.
};
