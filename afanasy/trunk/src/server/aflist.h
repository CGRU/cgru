#pragma once

#include <pthread.h>

#include "../libafanasy/afnode.h"

class AfListIt;
class AfContainer;

/// List of Afanasy's nodes.
class AfList
{
public:
   AfList();   ///< Construct an empty list.
   ~AfList();

   inline bool isInitialized(){  return initialized;       }///< Whether list is successfully initialized.
   inline int  getCount()     {  return nodes_list.size(); }///< Get nodes quantity in list.

   void moveNodes( const std::vector<int32_t> * list, int type);
   enum MoveType{
      MoveUp,
      MoveDown,
      MoveTop,
      MoveBottom
   };

   void generateIds( af::MCGeneral & ids) const;

   friend class AfListIt;
   friend class AfContainer;

protected:
   void add( af::Node *node);    ///< Add node to list.
   void remove( af::Node *node); ///< Remove node from list.

private:
   bool initialized;          ///< Whether list is successfully initialized.
   pthread_rwlock_t rwlock;   ///< List mutex.

   NodesList nodes_list;      ///< Nodes list.

   void sortPriority( af::Node * node);   ///< Sort nodes by priority.
};
