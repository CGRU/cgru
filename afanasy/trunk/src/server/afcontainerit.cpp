#include "afcontainerit.h"
#include "afcontainer.h"

#include "afcommon.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

AfContainerLock::AfContainerLock( AfContainer* afcontainer, int locktype):
   container(afcontainer),
   type( locktype)
{
   switch( type)
   {
      case AfContainer::READLOCK:
      {
         if( pthread_rwlock_rdlock( &container->rwlock) != 0)
            AFERRPE("AfContainerLock::AfContainerLock: pthread_rwlock_rdlock:");
         break;
      }
      case AfContainer::WRITELOCK:
      {
         if( pthread_rwlock_wrlock( &container->rwlock) != 0)
            AFERRPE("AfContainerLock::AfContainerLock: pthread_rwlock_wrlock:");
         break;
      }
      default:
         AFERROR("AfContainerLock::AfContainerLock: invalid lock type.");
   }
}
AfContainerLock::~AfContainerLock()
{
   if( pthread_rwlock_unlock( &container->rwlock) != 0)
      AFERRPE("AfContainerLock::~AfContainerLock: pthread_rwlock_unlock:");
}

AfContainerIt::AfContainerIt( AfContainer* afContainer, bool skipZombies):
   container( afContainer),
   byPassZombies( skipZombies)
{
   reset();
}

AfContainerIt::~AfContainerIt()
{
}

void AfContainerIt::reset()
{
   node = container->first_ptr;
   if( node == NULL) return;
   if( byPassZombies )
   {
      while( node->zombie)
      {
         node = node->next_ptr;
         if( node == NULL) return;
      }
   }
}

void AfContainerIt::next()
{
   if( node == NULL) return;
   node = node->next_ptr;
   if( node == NULL) return;
   if( byPassZombies )
   {
      while( node->zombie )
      {
         node = node->next_ptr;
         if( node == NULL) return;
      }
   }
}

af::Node* AfContainerIt::get( int id)
{
   if( id < 1 )
   {
      AFCommon::QueueLogError("AfContainerIt::get(): invalid id = " + af::itos(id));
      return NULL;
   }
   if( id >= container->size)
   {
      AFCommon::QueueLogError("AfContainerIt::get(): id=" + af::itos(id) + " >= size=" + af::itos(container->size));
      return NULL;
   }

   node = container->nodesTable[id];

   if( node == NULL )
   {
      AFCommon::QueueLogError("AfContainerIt::get(): node == NULL : id = " + af::itos(id));
      return NULL;
   }
   if( node->zombie)
   {
      AFCommon::QueueLogError("AfContainerIt::get(): node is zombie ( id = %d ) \n" + af::itos(id));
      return NULL;
   }

   return node;
}
