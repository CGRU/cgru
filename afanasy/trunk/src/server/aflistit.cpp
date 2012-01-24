#include "aflistit.h"

#include <stdio.h>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

AfListIt::AfListIt( AfList* aflist):
   node( NULL),
   list( aflist)
{
//	aflist->ReadLock();
   reset();
}

AfListIt::~AfListIt()
{
//	list->ReadUnlock();
}

void AfListIt::next()
{
   node = NULL;
   it++;
   if( it == it_end)
   {
//printf("AfListIt::next: it == it_end\n");
      return;
   }
   while( (*it)->zombie )
   {
      if( ++it == it_end )
      {
//printf("AfListIt::next: ++it == it_end\n");
         return;
      }
   }
   node = *it;
}

void AfListIt::reset()
{
   it = list->nodes_list.begin();
   it_end = list->nodes_list.end();
   if( it == it_end) return;
   while( (*it)->zombie ) if( ++it == it_end ) return;
   node = *it;
}
