#include "afcontainerit.h"
#include "afcontainer.h"

#include "afcommon.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

AfContainerLock::AfContainerLock( AfContainer* afcontainer, LockType locktype):
   container(afcontainer),
   type(locktype)
{
   switch( type )
   {
      case READLOCK:
      {
         container->ReadLock();
         break;
      }
      case WRITELOCK:
      {
         container->WriteLock();
         break;
      }
      default:
         AFERROR("AfContainerLock::AfContainerLock: invalid lock type.");
   }
}

AfContainerLock::~AfContainerLock()
{
   if( type == READLOCK )
      container->ReadUnlock();
   else
   {
      assert( type == WRITELOCK );
      container->WriteUnlock();
   }
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
