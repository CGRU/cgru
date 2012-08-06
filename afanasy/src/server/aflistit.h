#pragma once

#include "../libafanasy/afnode.h"

#include "aflist.h"

/// AFlist nodes iterator.
class AfListIt
{
public:
/// Create iterator for given \c aflist .
   AfListIt( AfList* i_aflist);
   ~AfListIt();

   inline AfNodeSrv * node() { return m_node; }   ///< Get current node.
   void next();   ///< Set iterator to next node.
   void reset();  ///< Reset iterator to initial position.

protected:
   AfNodeSrv* m_node;                              ///< Current node.

private:
   AfList* m_list;                 ///< Iterator's list.
   std::list<AfNodeSrv*>::iterator m_it;       ///< First node iterator.
   std::list<AfNodeSrv*>::iterator m_it_end;   ///< Last node iterator.
};
