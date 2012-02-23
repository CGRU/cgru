#pragma once

#include "dlMutex.h"
#include "dlThread.h"

#ifdef WINNT
#include <winsock2.h>
#include <windows.h>
#else
#include <semaphore.h>
#endif

#include <string>

namespace afqt { class QMsgQueue; }

namespace af
{
void thread_entry_point( void *i_parameter );

/// Queue item
class AfQueueItem
{
public:
   AfQueueItem();
   virtual ~AfQueueItem();
   friend class AfQueue;
   friend class afqt::QMsgQueue;
private:
   mutable AfQueueItem * next_ptr;
};

/*
   This class implements a messague queue using waiting semaphores.
*/
class AfQueue
{
   friend void thread_entry_point( void *i_parameter );

public:
   enum StartTread
   {
      e_start_thread,
      e_no_thread
   };

   enum WaitMode
   {
      e_wait,
      e_no_wait
   };

public:
   AfQueue( const std::string & QueueName, StartTread i_start_thread);
   virtual ~AfQueue();

   void lock();
   void unlock();

   bool isInitialized( void ) const { return true; }

protected:

/// Return first item from queue. BLOCKING FUNCTION if \c block==e_wait .
   AfQueueItem* pop( WaitMode i_block );
   bool push( AfQueueItem* item, bool i_front=false );

   /// Called from run thead to process item just poped from queue
   virtual void processItem( AfQueueItem* item) = 0;

   /*
      This function is called from a thread and waits on our
      counting semaphore.
   */
   void run();


   std::string name;

private:
   /* Mutex to lock access to the actual queue (when adding elements) */
   DlMutex m_mutex;

   /* Thread reading from the queue (and waitong on the semaphore. */
   DlThread m_thread;

   /* = true if we started a thread for this queue. */
   bool m_thread_started;

   /* A semaphore on which to sleep when there are no items in the queue. 
      We need a pointer and a semaphore because MacOS X systems use pointers
      on sempahore openb but Linux doesn't (this simplifies code, see
      implementation). */
#ifndef WINNT
   sem_t semcount;
   sem_t *semcount_ptr;
#else
   HANDLE semaphore;
#endif

   int count;           ///< Number of items in queue.

   AfQueueItem* firstPtr;  ///< Pointer to first item in queue.
   AfQueueItem* lastPtr;   ///< Pointer to last item in queue.
};

} // namespace af
