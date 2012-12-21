#include "aflist.h"

#include <stdio.h>

#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/dlScopeLocker.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"


/* FIXME: Using a list to manage priorities is not very good.
   Just use an AVL tree! An stl sorted container for example.
*/

AfList::~AfList()
{
	std::list<AfNodeSrv*>::iterator it = nodes_list.begin();
	std::list<AfNodeSrv*>::iterator end_it = nodes_list.end();

   while( it != end_it)
	  (*it++)->m_lists.remove( this);
}

int AfList::add( AfNodeSrv *node)
{
//   m_rw_lock.WriteLock();

   int index = -1;

	std::list<AfNodeSrv*>::iterator it = std::find( nodes_list.begin(), nodes_list.begin(), node);

   if( *it == node )
   {
      AFERROR("AfList::add: node already exists.\n");
      return index;
   }
   else
   {
      if( nodes_list.size() != 0 )
      {
         std::list<AfNodeSrv*>::iterator it = nodes_list.begin();
         std::list<AfNodeSrv*>::iterator end_it = nodes_list.end();
         bool lessPriorityFounded = false;
         while( it != end_it)
         {
            index++;
            if( **it >= *node ) { it++; continue;}

            nodes_list.insert( it, node);

            lessPriorityFounded = true;
            break;
         }
         if( lessPriorityFounded == false )
            nodes_list.push_back( node);
      }
      else
         nodes_list.push_back( node);

      index++;
      node->m_lists.push_back( this);
   }

   return index;
}

void AfList::remove( AfNodeSrv *node)
{
   nodes_list.remove(node);
   node->m_lists.remove(this);
}

int AfList::sortPriority( AfNodeSrv * i_node)
{
   if( nodes_list.size() < 2 )
   {
      return -1;
   }

   int index = -1;

   nodes_list.remove( i_node);
   std::list<AfNodeSrv*>::iterator it = nodes_list.begin();
   std::list<AfNodeSrv*>::iterator end_it = nodes_list.end();
   bool lessPriorityFounded = false;

   while( it != end_it)
   {
      index++;
      if( **it >= *i_node ) { it++; continue;}
      nodes_list.insert( it, i_node);
      lessPriorityFounded = true;
      break;
   }

   if( lessPriorityFounded == false )
   {
      index++;
      nodes_list.push_back( i_node);
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
	std::list<AfNodeSrv*> move_list;
	std::list<AfNodeSrv*>::iterator it_begin = nodes_list.begin();
	std::list<AfNodeSrv*>::iterator it_end   = nodes_list.end();
	std::list<AfNodeSrv*>::iterator it = it_begin;
   while( it != it_end)
   {
      for( unsigned n = 0; n < i_list.size(); n++)
      {
         if( (*it)->m_node->m_id == i_list[n] )
         {
#ifdef AFOUTPUT
//printf("Founded a node \"%s\"-%d\n", (*it)->getName().c_str(), (*it)->m_node->m_id);
printf("Founded a node \"%s\"-%d\n", (*it)->m_node->m_name.c_str(), (*it)->m_node->m_id );
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
	std::list<AfNodeSrv*>::iterator it_move_begin = move_list.begin();
	std::list<AfNodeSrv*>::iterator it_move_end   = move_list.end();
	std::list<AfNodeSrv*>::iterator it_move;
   if(( i_type == MoveDown) || ( i_type == MoveTop)) it_move = it_move_end;
   else it_move = it_move_begin;

   for(;;)
   {
      AfNodeSrv * node;
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
//printf("Processing node \"%s\"-%d\n", node->getName().c_str(), node->getId());
printf("Processing node \"%s\"-%d\n",  node->m_node->m_name.c_str(), node->m_node->m_id );
#endif
      std::list<AfNodeSrv*>::iterator it_insert = nodes_list.begin();
      while( it_insert != it_end)
      {
         if((*it_insert) == node) break;
         it_insert++;
      }
      if( it_insert == it_end)
      {
         AFERRAR("AfList::moveNodes: Lost node - \"%s\" - %d.\n", node->m_node->m_name.c_str(), node->m_node->m_id);
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
            if( **it_insert != *node)
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
            if( **it_insert != *node )
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
               if( **it_insert != *node )
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
               if( **it_insert != *node ) break;
               it_insert++;
               if( it_insert == it_end) break;
            }
            break;
         }
      }
      if( it_insert == it_end)
      {
#ifdef AFOUTPUT
printf("Pushing node back\n");
#endif
         nodes_list.remove( node);
         nodes_list.push_back( node);
         continue;
      }
      AfNodeSrv * node_move = (*it_insert);
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
//printf("Inserting at \"%s\"-%d\n", node_move->getName().c_str(), node_move->getId());
printf("Inserting at \"%s\"-%d\n", node_move->m_node->m_name.c_str(), node_move->m_node->m_id );
#endif
      nodes_list.remove( node);
      nodes_list.insert( it_insert, node);
   }
}

const std::vector<int32_t> AfList::generateIdsList() const
{
	std::vector<int32_t> ids;
	std::list<AfNodeSrv*>::const_iterator it = nodes_list.begin();
	while( it != nodes_list.end())
		ids.push_back( (*(it++))->m_node->m_id);

	return ids;
}

