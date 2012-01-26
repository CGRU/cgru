#pragma once

#include "../libafanasy/afnode.h"

#include "aflist.h"

/// AFlist nodes iterator.
class AfListIt
{
public:
/// Create iterator for given \c aflist .
   AfListIt( AfList* aflist);
   ~AfListIt();

   void next();   ///< Set iterator to next node.
   void reset();  ///< Reset iterator to initial position.

protected:
   inline af::Node* object() { return node; }   ///< Get current node.
   af::Node* node;                              ///< Current node.

private:
   AfList* list;                 ///< Iterator's list.
   std::list<af::Node*>::iterator it;       ///< First node iterator.
   std::list<af::Node*>::iterator it_end;   ///< Last node iterator.
};
