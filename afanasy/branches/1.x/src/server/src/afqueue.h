#pragma once

//#define MACOSX

#ifdef MACOSX
#include <QtCore/QSemaphore>
#include <QtCore/QMutex>
#else
#include "pthread.h"
#include "semaphore.h"
#endif

#include <QtCore/QString>

class AfQueueItem;

/// Simple FIFO items queue, with pthread mutex and semaphore counter for \c push() and \c pop() operations.
class AfQueue
{
public:
   AfQueue( const QString & QueueName);    ///< Constructor initialize mutex and set \c initialized variable to \c true
   ~AfQueue();   ///< Desctuctor deletes all existing items in queue

   inline bool isInitialized(){ return initialized;}  ///< Return \c true if queue successfully initialized.
   inline int  size()         { return count;      }  ///< Ruturn number of items in queue.

   void run();
   void quit();

   void lock();
   void unlock();

protected:
/// Return first item from queue. BLOCKING FUNCTION if \c block==true .
   AfQueueItem* pop( bool block = true);

   bool push( AfQueueItem* item);   ///< Push items to queue back.

   virtual void processItem( AfQueueItem* item) const;

private:
   QString name;

#ifdef MACOSX
   QMutex      q_mutex;      ///< Mutex for \c push() and \c pop() operations.
   QSemaphore  q_semaphore;  ///< Messages count semaphore
#else
   pthread_mutex_t mutex;  ///< Mutex for \c push() and \c pop() operations.
   sem_t semcount;         ///< Items count semaphore
#endif

   int count;           ///< Number of items in queue.

   AfQueueItem* firstPtr;  ///< Pointer to first item in queue.
   AfQueueItem* lastPtr;   ///< Pointer to last item in queue.

   bool locked;

   bool initialized;    ///< \c TRUE if class sucessfully initialized.
};
//#undef MACOSX
