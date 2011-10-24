#include "afqueue.h"

#include <stdio.h>

extern bool running;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

AfQueueItem::AfQueueItem(): next_ptr( NULL) {}
AfQueueItem::~AfQueueItem() {}

AfQueue::AfQueue( const std::string & QueueName):
   name( QueueName),
   count( 0),
   firstPtr( NULL),
   lastPtr( NULL),
   locked( false),
   initialized( false)
{
#ifndef MACOSX
   if( pthread_mutex_init( &mutex, NULL) != 0)
   {
      AFERRPE("AfQueue::AfQueue(): pthread_mutex_init:")
      return;
   }
   if( sem_init( &semcount, 0, 0) != 0)
   {
      AFERRPE("AfQueue::AfQueue(): sem_init:")
      return;
   }
#endif
   initialized = true;
}

AfQueue::~AfQueue()
{
   AFINFA("AfQueue::~AfQueue(): %s", name.c_str())
   while( firstPtr != NULL)
   {
      AfQueueItem* item = firstPtr;
      firstPtr = item->next_ptr;
      delete item;
   }
}

void AfQueue::lock()
{
   if( locked )
   {
      AFERRAR("AfQueue::lock: '%s' already locked.", name.c_str())
      return;
   }
#ifdef MACOSX
   q_mutex.lock();
#else
   pthread_mutex_lock( &mutex);
#endif
   locked = true;
}

void AfQueue::unlock()
{
   if( false == locked )
   {
      AFERRAR("AfQueue::lock: '%s' not locked.", name.c_str())
      return;
   }
#ifdef MACOSX
   q_mutex.unlock();
#else
   pthread_mutex_unlock( &mutex);
#endif
   locked = false;
}

bool AfQueue::push( AfQueueItem* item, bool front)
{
   if( item == NULL)
   {
#ifdef MACOSX
      q_semaphore.release();
#else
      sem_post( &semcount);
#endif
      return false;
   }
   item->next_ptr = NULL;

//BEGIN mutex
#ifdef MACOSX
   if( false == locked ) q_mutex.lock();
#else
   if( false == locked ) pthread_mutex_lock( &mutex);
#endif
   {
      if( front)
      {
         item->next_ptr = firstPtr;
         if( count == 0) lastPtr = item;
         firstPtr = item;
      }
      else
      {
         if( count == 0) firstPtr = item;
         else lastPtr->next_ptr = item;
         lastPtr = item;
      }
      count++;
   }
#ifdef MACOSX
   q_semaphore.release();
   if( false == locked ) q_mutex.unlock();
#else
   if( sem_post( &semcount) != 0)
   {
      AFERRPE("AfQueue::push: sem_post:")
   }
   if( false == locked ) pthread_mutex_unlock( &mutex);
#endif
//END mutex

   AFINFA("Msg* AfQueue::push: item=%p, count=%d", item, count)
   return true;
}

AfQueueItem* AfQueue::pop( bool block)
{
   AfQueueItem* item = NULL;

#ifdef MACOSX
   bool semresult = true;
   if( block)        q_semaphore.acquire();
   else semresult =  q_semaphore.tryAcquire();
   if( semresult == false ) return NULL;
#else
   int semresult;
   if( block)  semresult = sem_wait(    &semcount );
   else        semresult = sem_trywait( &semcount );
   if( semresult != 0 ) return NULL;
#endif

//BEGIN mutex
#ifdef MACOSX
   QMutexLocker lock( &q_mutex);
#else
   pthread_mutex_lock( &mutex);
#endif
   if((count > 0) && (firstPtr != NULL))
   {
      item = firstPtr;
      firstPtr = item->next_ptr;
      item->next_ptr = NULL;
      count--;
   }
#ifndef MACOSX
   pthread_mutex_unlock( &mutex);
#endif
//END mutex

   AFINFA("Msg* AfQueue::pop: item=%p, count=%d", item, count)
   return item;
}

void AfQueue::run()
{
while( running)
{
   AfQueueItem * item = pop();
   if( running == false)
   {
      if( item != NULL ) delete item;
      return;
   }
   if( item == NULL) continue;

   // If item not needed any more it must be deleted in this virtual function:
   processItem( item);
}
AFINFO("AfQueue::run: finished.")
}

void AfQueue::quit()
{
   push( NULL);
}
/*
void AfQueue::processItem( AfQueueItem* item)
{
   AFERRAR("AfQueue::processItem: \"%s\" - Invalid call", name.c_str())
}
*/
