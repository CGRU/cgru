#pragma once

#include <pthread.h>

#include "../libafanasy/afnode.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "msgaf.h"

class AfContainer;
class AfContainerIt;

/// Afanasy nodes container.
class AfContainer
{
public:
/// Initialize container for \c maximumsize nodes.
   AfContainer( std::string ContainerName, int MaximumSize);
   ~AfContainer();

   inline bool isInitialized() {  return initialized;}///< Whether container was successfully initialized.

/// Generate MCAfNodes message with all stored Nodes.
   MsgAf* generateList( int type);

/// Generate MCAfNodes message with Nodes with provided ids.
   MsgAf* generateList( int type, const af::MCGeneral & mcgeneral);

   bool setZombie( int id);

/// Free zombie nodes memory.
   void freeZombies();

/// Refresh container nodes.
   void refresh( AfContainer * pointer, MonitorContainer * monitoring);

/// Set some attributes of some nodes that came from message class.
   void action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring);

   friend class AfContainerIt;
   friend class AfContainerLock;
   friend class AfList;
   friend class AfListIt;

enum LOCKTYPE  ///< Type of container lock.
{
   READLOCK,   ///< Read lock.
   WRITELOCK,  ///< Write lock.
};

protected:
   int add( af::Node *node);   ///< Add node to container.

private:
   pthread_rwlock_t rwlock;   ///< Read|Write lock.
   int count;                 ///< Number of nodes in container.
   int size;                  ///< Container size ( maximun number of node can be stored).
   af::Node * first_ptr;      ///< Pointer to first node.
   af::Node * last_ptr;       ///< Pointer to last node.
   af::Node ** nodesTable;          ///< Nodes pointers.
   bool initialized;          ///< Whether container was successfully initialized.
   std::string name;          ///< Container name.

private:
/// Set some node attribute that came from message class.
   void action( af::Node * node, const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring);

/// Sort nodes list according to priority, called when priority attribute of any node changes.
   void sortPriority( af::Node * node);
};
