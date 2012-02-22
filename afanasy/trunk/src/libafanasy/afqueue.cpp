#include "afqueue.h"
#include "dlScopeLocker.h"

#include <stdio.h>
#include <assert.h>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

bool AFRunning = true;

using namespace af;

/*
   This is a simple stub to call the "Run" method in the
   AfQueue.
*/
void af::thread_entry_point( void *i_parameter )
{
   af::AfQueue *queue = (af::AfQueue *) i_parameter;
   queue->run();
}

AfQueueItem::AfQueueItem(): next_ptr( NULL) {}
AfQueueItem::~AfQueueItem() {}

AfQueue::AfQueue( const std::string &i_QueueName, StartTread i_start_thread ):
   name(i_QueueName),
   count(0),
   firstPtr(NULL),
   lastPtr(NULL),
   m_thread_started(i_start_thread)
{
#ifdef WINNT
	semaphore = CreateSemaphore( NULL, 1024, 1024, NULL);
	if (semaphore == NULL) 
    {
		AFERROR("CreateSemaphore error in queue '%s'", i_QueueName.c_str());
        return;
    }
#elif defined(MACOSX)
   /* We carate an exclusive semaphore and give read and write
      persmission to this owner. */
   semcount_ptr = sem_open( i_QueueName.c_str(), O_CREAT, 0600, 0);

   if( semcount_ptr == SEM_FAILED )
   {
      perror( "sem_open() failing in AfQueue ctor" );
      return;
   }
#else
   semcount_ptr = &semcount;
   if( sem_init( semcount_ptr, 0, 0) != 0)
   {
      perror( "sem_open() failed in AfQueue ctor" );
      return;
   }
#endif

   if( i_start_thread == e_start_thread )
   {
      /* Start the thread which waits for elements in the queue. */
      m_thread.Start( thread_entry_point, this );
   }
}

AfQueue::~AfQueue()
{
   /* At this point there is only no possible concurrent calls to
      the AfQueu so we don't need the mutex. We still try to lock
      the mutex to perform a sanity check. */

   bool locked = m_mutex.TryLock();
   if( !locked )
   {
      assert( !"AfQueue dtor called while queue is locked." );
      return;
   }

#ifdef WINNT
//   CloseHandle( semaphore);
//#else
   sem_post( semcount_ptr);
#endif

   AFINFA("AfQueue::~AfQueue(): %s", name.c_str())

   while( firstPtr != NULL)
   {
      AfQueueItem* item = firstPtr;
      firstPtr = item->next_ptr;
      delete item;
   }

#ifdef WINNT
   CloseHandle( semaphore);
#else
   sem_close( semcount_ptr );
#endif

   m_mutex.Unlock();

   if( m_thread_started )
   {
      m_thread.Cancel();
      m_thread.Join();
   }
}

void AfQueue::lock()
{
   m_mutex.Lock();
}

void AfQueue::unlock()
{
   m_mutex.Unlock();
}

bool AfQueue::push( AfQueueItem* item, bool i_front )
{
	assert( item );

   item->next_ptr = NULL;

   {
      DlScopeLocker lock( &m_mutex );

      if( i_front )
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

   /*
      Now that we added a new element to this list, we can increase
      the semaphore count so that waiting processes can wake up.
   */

#ifndef _WIN32
   if( sem_post(semcount_ptr) == -1 )
   {
      perror( "sem_post() failed in AfQueue::~AfQueue()" );
   }
#endif

   AFINFA("Msg* AfQueue::push: item=%p, count=%d", iteueuem, count);

   return true;
}

AfQueueItem* AfQueue::pop( WaitMode i_mode )
{
   AfQueueItem* item = NULL;

#ifndef _WIN32
   int semresult = i_mode==e_wait ?
      sem_wait(semcount_ptr) : sem_trywait(semcount_ptr);

   if( semresult == -1 )
	{
		if( i_mode == e_wait )
		{
			perror( "sem_wait() failed in AfQueue::pop" );
			return 0x0;
		}
		else if( errno != EAGAIN )
		{
			perror( "sem_trywait() failed in AfQueue::pop" );
			return 0x0;
		}
	}

#endif

   DlScopeLocker lock( &m_mutex );
   if((count > 0) && (firstPtr != NULL))
   {
      item = firstPtr;
      firstPtr = item->next_ptr;
      item->next_ptr = NULL;
      count--;
   }

   AFINFA("Msg* AfQueue::pop: item=%p, count=%d", item, count)
   return item;
}

/*
   This is the main method that will treat all in the incoming messages.
   It will simply wait on the counting semaphore when no messages are 
   available and this means we take no CPU.
*/
void AfQueue::run()
{
   AFINFA( "Queue '%s' just came to life!", name.c_str() );

   while( AFRunning )
   {

      AfQueueItem * item = pop( e_wait );

      if( AFRunning == false )
      {
         delete item;
         return;
      }

      assert( item );

      // Item must be deleted in this virtual function.
      processItem( item );

      /*
         This is a safe concellation point.
      */
      if( m_thread_started )
         m_thread.TestCancel();
   }
   
   AFINFA("AfQueue::run is finished for queue '%s'.", name.c_str() )
}
