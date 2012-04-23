#pragma once

#include "../libafanasy/name_af.h"

#include "name_afqt.h"

#include <QtCore/QSemaphore>
#include <QtCore/QMutex>

/// Simple FIFO message queue, with pthread mutex and semaphore counter for \c push() and \c pop() operations.
class afqt::QMsgQueue
{
public:
   QMsgQueue();    ///< Constructor initialize mutex and set \c initialized variable to \c true
   ~QMsgQueue();   ///< Desctuctor deletes all existing messages in queue

/// Return first message from queue. BLOCKING FUNCTION if \c block==true .
   af::Msg * pop( bool block = true);

   bool push( af::Msg * msg);   ///< Push message to queue back.

   inline int       getCount() const { return count;   }
   inline af::Msg * getFirst() const { return firstPtr;}

private:

   QMutex      mutex;      ///< Mutex for \c push() and \c pop() operations.
   QSemaphore  semaphore;  ///< Messages count semaphore

   af::Msg * firstPtr;  ///< Pointer to first message in queue.
   af::Msg * lastPtr;   ///< Pointer to last message in queue.
   int count;           ///< Number of messages in queue.
};
