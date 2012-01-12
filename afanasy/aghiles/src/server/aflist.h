#pragma once

#include "../libafanasy/afnode.h"
#include "../libafanasy/DlRWLock.h"

class AfListIt;
class AfContainer;

/// List of Afanasy's nodes.
class AfList
{
public:
   AfList()
   {
   }

   ~AfList();

   int getCount()
   {
      return nodes_list.size();
   }

   void moveNodes( const std::vector<int32_t> * list, int type);

   enum MoveType{
      MoveUp,
      MoveDown,
      MoveTop,
      MoveBottom
   };

   void generateIds( af::MCGeneral & ids) const;

   friend class AfContainer;
   friend class AfListIt;

   void ReadLock( void ) { m_rw_lock.ReadLock(); }
   void ReadUnlock( void ) { m_rw_lock.ReadUnlock(); }

protected:
   int add( af::Node *node);    ///< Add node to list.
   void remove( af::Node *node); ///< Remove node from list.

private:
   int sortPriority( af::Node * node);   ///< Sort nodes by priority.

private:
   DlRWLock m_rw_lock;

   NodesList nodes_list;      ///< Nodes list.
};
