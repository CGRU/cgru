#include "aflist.h"

#include <stdio.h>

#include "../libafanasy/msgclasses/mcgeneral.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

AfList::AfList():
   initialized( false)
{
   if( pthread_rwlock_init( &rwlock, NULL) != 0)
   {
      AFERRPE("AfList::AfList:");
      return;
   }
   initialized = true;
}

AfList::~AfList()
{
AFINFO("AfList::~AfList:\n");
   NodesList::iterator it = nodes_list.begin();
   NodesList::iterator end_it = nodes_list.end();
   while( it != end_it) (*it++)->lists.remove( this);
}

int AfList::add( af::Node *node)
{
   int index = -1;
//BEGIN mutex
   if( pthread_rwlock_wrlock( &rwlock) != 0)
      AFERRPE("AfList::add: pthread_rwlock_wrlock:");
/*-------------------------------------------------------------*/
   NodesList::iterator it = std::find( nodes_list.begin(), nodes_list.begin(), node);
   if( *it == node )
   {
      AFERROR("AfList::add: node already exists.\n");
      return index;
   }
   else
   {
      if( nodes_list.size() != 0 )
      {
         NodesList::iterator it = nodes_list.begin();
         NodesList::iterator end_it = nodes_list.end();
         bool lessPriorityFounded = false;
         while( it != end_it)
         {
            index++;
            if( *(*it) >= *node ) { it++; continue;}

            nodes_list.insert( it, node);

            lessPriorityFounded = true;
            break;
         }
         if( lessPriorityFounded == false ) nodes_list.push_back( node);
      }
      else
         nodes_list.push_back( node);

      index++;
      node->lists.push_back( this);
   }
/*-------------------------------------------------------------*/
   if( pthread_rwlock_unlock( &rwlock) != 0)
      AFERRPE("AfList::add: pthread_rwlock_unlock:");
//END mutex
   return index;
}

void AfList::remove( af::Node *node)
{
//BEGIN mutex
   if( pthread_rwlock_wrlock( &rwlock) != 0)
      AFERRPE("AfList::remove: pthread_rwlock_wrlock:");
/*-------------------------------------------------------------*/
   nodes_list.remove( node);
   node->lists.remove( this);
/*-------------------------------------------------------------*/
   if( pthread_rwlock_unlock( &rwlock) != 0)
      AFERRPE("AfList::remove: pthread_rwlock_unlock:");
//END mutex
}

int AfList::sortPriority( af::Node * node)
{
   int index = -1;
   if( nodes_list.size() < 2 ) return index;

//BEGIN mutex
   if( pthread_rwlock_wrlock( &rwlock) != 0)
      AFERRPE("AfList::add: pthread_rwlock_wrlock:");
/*-------------------------------------------------------------*/
      nodes_list.remove( node);
      NodesList::iterator it = nodes_list.begin();
      NodesList::iterator end_it = nodes_list.end();
      bool lessPriorityFounded = false;
      while( it != end_it)
      {
         index++;
         if( *(*it) >= *node ) { it++; continue;}
         nodes_list.insert( it, node);
         lessPriorityFounded = true;
         break;
      }
      if( lessPriorityFounded == false )
      {
         index++;
         nodes_list.push_back( node);
      }
/*-------------------------------------------------------------*/
   if( pthread_rwlock_unlock( &rwlock) != 0)
      AFERRPE("AfList::remove: pthread_rwlock_unlock:");
//END mutex

   return index;
}

void AfList::moveNodes( const std::vector<int32_t> * list, int type)
{
#ifdef AFOUTPUT
printf("AfList::moveNodes:\n");
#endif
//
//    ensure in proper type
//
   switch ( type)
   {
      case MoveUp: case MoveDown: case MoveTop: case MoveBottom: break;
      default:
         AFERRAR("AfList::moveNodes: Invalid type = %d\n", type);
         return;
   }
//
//    return if no nodes ids to move
//
   if( list->size() == 0)
   {
      AFERROR("AfList::moveNodes: Move nodes ids list is empty.\n");
      return;
   }

//
//    creating move nodes list
//
   NodesList move_list;
   NodesList::iterator it_begin = nodes_list.begin();
//   NodesList::iterator it_back  = nodes_list.back();
   NodesList::iterator it_end   = nodes_list.end();
   NodesList::iterator it = it_begin;
   while( it != it_end)
   {
      for( unsigned n = 0; n < list->size(); n++)
      {
         if( (*it)->getId() == (*list)[n] )
         {
#ifdef AFOUTPUT
printf("Founded a node \"%s\"-%d\n", (*it)->getName().toUtf8().data(), (*it)->getId());
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
   NodesList::iterator it_move_begin = move_list.begin();
   NodesList::iterator it_move_end   = move_list.end();
   NodesList::iterator it_move;
   if((type == MoveDown) || (type == MoveTop)) it_move = it_move_end;
   else it_move = it_move_begin;

   for(;;)
   {
      af::Node * node;
      if((type == MoveDown) || (type == MoveTop))
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
printf("Processing node \"%s\"-%d\n", node->getName().toUtf8().data(), node->getId());
#endif
      NodesList::iterator it_insert = nodes_list.begin();
      while( it_insert != it_end)
      {
         if((*it_insert) == node) break;
         it_insert++;
      }
      if( it_insert == it_end)
      {
         AFERRAR("AfList::moveNodes: Lost node - \"%s\" - %d.\n", node->getName().c_str(), node->getId());
         continue;
      }

      switch ( type)
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
            if( **it_insert != *node)
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
               if( **it_insert != *node)
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
//            while((it_insert++ != it_end) && (**it_insert == *node));
            for(;;)
            {
               if( **it_insert != *node) break;
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
      af::Node * node_move = (*it_insert);
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
printf("Inserting at \"%s\"-%d\n", node_move->getName().toUtf8().data(), node_move->getId());
#endif
      nodes_list.remove( node);
      nodes_list.insert( it_insert, node);
   }
}

void AfList::generateIds( af::MCGeneral & ids) const
{
   NodesList::const_iterator it = nodes_list.begin();
   while( it != nodes_list.end()) ids.addId( (*(it++))->getId());
}
