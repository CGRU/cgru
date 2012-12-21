#include "aflistit.h"

#include <stdio.h>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

AfListIt::AfListIt( AfList* i_aflist):
   m_node( NULL),
   m_list( i_aflist)
{
   reset();
}

AfListIt::~AfListIt()
{
}

void AfListIt::next()
{
   m_node = NULL;
   m_it++;
   if( m_it == m_it_end)
   {
//printf("AfListIt::next: it == it_end\n");
      return;
   }
//   while( (*m_it)->isZombie())
   while( (*m_it)->m_node->isZombie())
   {
      if( ++m_it == m_it_end )
      {
//printf("AfListIt::next: ++it == it_end\n");
         return;
      }
   }
   m_node = *m_it;
}

void AfListIt::reset()
{
   m_it = m_list->nodes_list.begin();
   m_it_end = m_list->nodes_list.end();
   if( m_it == m_it_end)
   {
       return;
   }
//   while( (*m_it)->isZombie())
   while( (*m_it)->m_node->isZombie())
   {
       if( ++m_it == m_it_end )
       {
           return;
       }
   }
   m_node = *m_it;
}
