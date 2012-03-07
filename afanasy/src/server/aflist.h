#pragma once

#include "../libafanasy/afnode.h"
//#include "../libafanasy/dlRWLock.h"

class AfListIt;
class AfContainer;
class MonitorContainer;
class RenderAf;

/// List of Afanasy's nodes.
class AfList
{
public:
   AfList()
   {
   }

   ~AfList();

   inline int getCount() const { return nodes_list.size();}

   int add( af::Node *node);    ///< Add node to list.

   /// Solve nodes list:
   inline bool solve( af::Node::SolvingMethod i_method, RenderAf * i_render, MonitorContainer * i_monitoring)
                { return af::Node::solveList( nodes_list, i_method, i_render, i_monitoring);}

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

//   void ReadLock( void ) { m_rw_lock.ReadLock(); }
//   void ReadUnlock( void ) { m_rw_lock.ReadUnlock(); }

//protected:
   void remove( af::Node *node); ///< Remove node from list.

private:
   int sortPriority( af::Node * node);   ///< Sort nodes by priority.

private:
//   DlRWLock m_rw_lock;

   std::list<af::Node*> nodes_list;      ///< Nodes list.
};
