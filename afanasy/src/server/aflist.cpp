#include "aflist.h"

#include <stdio.h>

#include "../libafanasy/common/dlScopeLocker.h"

#include "../libafanasy/msgclasses/mcgeneral.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"


/* FIXME: Using a list to manage priorities is not very good.
   Just use an AVL tree! An stl sorted container for example.
*/

AfList::AfList()
{
}

AfList::~AfList()
{
	std::list<AfNodeSolve*>::iterator it = m_nodes_list.begin();
	std::list<AfNodeSolve*>::iterator end_it = m_nodes_list.end();

   while( it != end_it)
	  (*it++)->m_lists.remove( this);
}

int AfList::add( AfNodeSolve *node)
{
	int index = -1;

	for( std::list<AfNodeSolve*>::const_iterator it = m_nodes_list.begin(); it != m_nodes_list.end(); it++)
		if( *it == node )
		{
			AFERROR("AfList::add: node already exists.");
			return index;
		}

	if( m_nodes_list.size() != 0 )
	{
		std::list<AfNodeSolve*>::iterator it = m_nodes_list.begin();
		std::list<AfNodeSolve*>::iterator end_it = m_nodes_list.end();
		bool lessPriorityFound = false;
		while( it != end_it)
		{
			index++;
			if((*it)->priority() >= node->priority())
			{
				it++;
				continue;
			}

			m_nodes_list.insert( it, node);

			lessPriorityFound = true;
			break;
		}
		if( lessPriorityFound == false )
			m_nodes_list.push_back( node);
	}
	else
		m_nodes_list.push_back( node);

	index++;
	node->m_lists.push_back( this);

	return index;
}

void AfList::remove( AfNodeSolve * i_node)
{
   m_nodes_list.remove( i_node);
   i_node->m_lists.remove( this);
}

int AfList::sortPriority( AfNodeSolve * i_node)
{
   if( m_nodes_list.size() < 2 )
   {
      return -1;
   }

   int index = -1;

   m_nodes_list.remove( i_node);
   std::list<AfNodeSolve*>::iterator it = m_nodes_list.begin();
   std::list<AfNodeSolve*>::iterator end_it = m_nodes_list.end();
   bool lessPriorityFound = false;

   while( it != end_it)
   {
      index++;
      if((*it)->priority() >= i_node->priority())
	  {
		  it++;
		  continue;
	  }

      m_nodes_list.insert( it, i_node);
      lessPriorityFound = true;
      break;
   }

   if( lessPriorityFound == false )
   {
      index++;
      m_nodes_list.push_back( i_node);
   }

   return index;
}

void AfList::moveNodes( const std::vector<int32_t> & i_list, int i_type)
{
#ifdef AFOUTPUT
printf("AfList::moveNodes:\n");
#endif
//
//    ensure in proper type
//
   switch ( i_type)
   {
      case MoveUp: case MoveDown: case MoveTop: case MoveBottom: break;
      default:
         AFERRAR("AfList::moveNodes: Invalid type = %d\n", i_type);
         return;
   }
//
//    return if no nodes ids to move
//
   if( i_list.size() == 0)
   {
      AFERROR("AfList::moveNodes: Move nodes ids list is empty.\n");
      return;
   }

//
//    creating move nodes list
//
	std::list<AfNodeSolve*> move_list;
	std::list<AfNodeSolve*>::iterator it_begin = m_nodes_list.begin();
	std::list<AfNodeSolve*>::iterator it_end   = m_nodes_list.end();
	std::list<AfNodeSolve*>::iterator it = it_begin;
   while( it != it_end)
   {
      for( unsigned n = 0; n < i_list.size(); n++)
      {
         if( (*it)->node()->m_id == i_list[n] )
         {
#ifdef AFOUTPUT
printf("Found a node \"%s\"-%d\n", (*it)->node()->m_name.c_str(), (*it)->node()->m_id );
#endif
            move_list.push_back( *it);
            break;
         }
      }
      it++;
   }
//
//    return if it os no nodes to move
//
   if( move_list.size() == 0 )
   {
      AFERROR("AfList::moveNodes: Can't find nodes with such ids.\n");
      return;
   }

//
//    moving nodes in move list
//
	std::list<AfNodeSolve*>::iterator it_move_begin = move_list.begin();
	std::list<AfNodeSolve*>::iterator it_move_end   = move_list.end();
	std::list<AfNodeSolve*>::iterator it_move;
   if(( i_type == MoveDown) || ( i_type == MoveTop)) it_move = it_move_end;
   else it_move = it_move_begin;

   for(;;)
   {
      AfNodeSolve * node;
      if(( i_type == MoveDown) || ( i_type == MoveTop))
      {
         if( it_move == it_move_begin ) break;
         it_move--;
         node = *it_move;
      }
      else
      {
         if( it_move == it_move_end ) break;
         node = *it_move;
         it_move++;
      }
#ifdef AFOUTPUT
printf("Processing node \"%s\"-%d\n",  node->node()->getName().c_str(), node->node()->getId());
#endif
      std::list<AfNodeSolve*>::iterator it_insert = m_nodes_list.begin();
      while( it_insert != it_end)
      {
         if((*it_insert) == node) break;
         it_insert++;
      }
      if( it_insert == it_end)
      {
         AFERRAR("AfList::moveNodes: Lost node - \"%s\" - %d", node->node()->getName().c_str(), node->node()->getId());
         continue;
      }

      switch ( i_type)
      {
         case MoveUp:
         {
#ifdef AFOUTPUT
printf("AfList::MoveUp:\n");
#endif
            if( it_insert == it_begin)
            {
#ifdef AFOUTPUT
printf("Node is already at top.\n");
#endif
               continue;
            }
            it_insert--;
            if((*it_insert)->priority() != node->priority())
            {
#ifdef AFOUTPUT
printf("Node above has another priority\n");
#endif
               continue;
            }
            break;
         }
         case MoveDown:
         {
#ifdef AFOUTPUT
printf("AfList::MoveDown:\n");
#endif
            it_insert++;
            if( it_insert == it_end)
            {
#ifdef AFOUTPUT
printf("Node is already at bottom.\n");
#endif
               continue;
            }
            if((*it_insert)->priority() != node->priority())
            {
#ifdef AFOUTPUT
printf("Node below has another priority\n");
#endif
               continue;
            }
            it_insert++;
            break;
         }
         case MoveTop:
         {
#ifdef AFOUTPUT
printf("AfList::MoveTop:\n");
#endif
            if( it_insert == it_begin)
            {
#ifdef AFOUTPUT
printf("Node is already at top.\n");
#endif
               continue;
            }
            for(;;)
            {
               it_insert--;
               if((*it_insert)->priority() != node->priority())
               {
                  it_insert++;
                  break;
               }
               if( it_insert == it_begin) break;
            }
            break;
         }
         case MoveBottom:
         {
            it_insert++;
            if( it_insert == it_end)
            {
#ifdef AFOUTPUT
printf("Node is already at bottom.\n");
#endif
               continue;
            }
#ifdef AFOUTPUT
printf("AfList::MoveBottom:\n");
#endif
            for(;;)
            {
               if((*it_insert)->priority() != node->priority())
				   break;

               it_insert++;

               if( it_insert == it_end)
				   break;
            }
            break;
         }
      }
      if( it_insert == it_end)
      {
#ifdef AFOUTPUT
printf("Pushing node back\n");
#endif
         m_nodes_list.remove( node);
         m_nodes_list.push_back( node);
         continue;
      }
      AfNodeSolve * node_move = (*it_insert);
      if( node_move == NULL)
      {
         AFERROR("AfList::moveNodes: node_move == NULL\n");
         continue;
      }

      if( node_move == node )
      {
#ifdef AFOUTPUT
printf("Node is already at it's position.\n");
#endif
         continue;
      }

#ifdef AFOUTPUT
printf("Inserting at \"%s\"-%d\n", node_move->node()->getName().c_str(), node_move->node()->getId());
#endif
      m_nodes_list.remove( node);
      m_nodes_list.insert( it_insert, node);
   }
}

const std::vector<int32_t> AfList::generateIdsList() const
{
	std::vector<int32_t> ids;
	std::list<AfNodeSolve*>::const_iterator it = m_nodes_list.begin();
	while( it != m_nodes_list.end())
		ids.push_back( (*(it++))->node()->getId());

	return ids;
}



//////////////////////////////////////////////////////////////////////
/////////////////////        Iterator        /////////////////////////
//////////////////////////////////////////////////////////////////////


AfListIt::AfListIt( AfList* i_aflist):
	m_node( NULL),
	m_list( i_aflist)
{
	reset();
}

AfListIt::~AfListIt(){}

void AfListIt::next()
{
	m_node = NULL;
	m_it++;
	if( m_it == m_it_end)
		return;

	while( (*m_it)->node()->isZombie())
		if( ++m_it == m_it_end )
			return;

	m_node = *m_it;
}

void AfListIt::reset()
{
	m_it = m_list->m_nodes_list.begin();
	m_it_end = m_list->m_nodes_list.end();
	if( m_it == m_it_end)
		 return;

	while( (*m_it)->node()->isZombie())
		 if( ++m_it == m_it_end )
			  return;

	m_node = *m_it;
}

