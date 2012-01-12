#pragma once

#include <pthread.h>

#include "../libafanasy/afnode.h"

#include "afcontainer.h"

/// Afanasy nodes container locker.

/* FIXME: yes, but what is a node?? */
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
   AfContainerIt( AfContainer* afContainer, bool skipZombies);
   ~AfContainerIt();

   void next();   ///< Set iterator position on next node.
   void reset();  ///< Set iterator position on the first node.

   friend class AfContainer;

protected:
/// Get current node pointer. \c NULL returned if interator at end.
   inline const af::Node* object() const { return node; }

/// Get node with specitied id. \c NULL returned if there is no node with such id.
   af::Node* get( int id);

   af::Node* node; ///< Current node pointer.

private:
   AfContainer* container; ///< Container pointer.
   bool byPassZombies;     ///< Whether iterator will bypass zobmies in \c next() function.
};
