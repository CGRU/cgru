#pragma once

#include <QtCore/QSemaphore>
#include <QtCore/QMutex>

#include "name_afqt.h"

/// Simple FIFO message queue, with pthread mutex and semaphore counter for \c push() and \c pop() operations.
class afqt::QMsgQueue
{
public:
   QMsgQueue();    ///< Constructor initialize mutex and set \c initialized variable to \c true
   ~QMsgQueue();   ///< Desctuctor deletes all existing messages in queue

/// Return first message from queue. BLOCKING FUNCTION if \c block==true .
   QMsg* pop( bool block = true);

   bool push( QMsg* msg);   ///< Push message to queue back.

   inline int   getCount() const { return count;   }
   inline QMsg* getFirst() const { return firstPtr;}

private:

   QMutex      mutex;      ///< Mutex for \c push() and \c pop() operations.
   QSemaphore  semaphore;  ///< Messages count semaphore

   QMsg* firstPtr;  ///< Pointer to first message in queue.
   QMsg* lastPtr;   ///< Pointer to last message in queue.
   int count;           ///< Number of messages in queue.
};
