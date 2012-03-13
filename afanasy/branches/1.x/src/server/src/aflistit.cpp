#include "aflistit.h"

#include <stdio.h>

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

AfListIt::AfListIt( AfList* aflist):
   node( NULL),
   list( aflist)
{
//BEGIN mutex
   if( pthread_rwlock_rdlock( &list->rwlock) != 0)
      AFERRPE("AfListIt::AfListIt: pthread_rwlock_rdlock:");

   reset();
}

AfListIt::~AfListIt()
{
   if( pthread_rwlock_unlock( &list->rwlock) != 0)
      AFERRPE("AfListIt::~AfListIt: pthread_rwlock_unlock:");
//END mutex
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
