#include "qmsgqueue.h"

#include "qmsg.h"

using namespace afqt;

//#define AFOUTPUT
//#undef AFOUTPUT
#include <macrooutput.h>

QMsgQueue::QMsgQueue():
   firstPtr( NULL),
   lastPtr( NULL),
   count( 0)
{
}

QMsgQueue::~QMsgQueue()
{
AFINFO("QMsgQueue::~QMsgQueue():\n");
   while( firstPtr != NULL)
   {
      QMsg* msg = firstPtr;
      firstPtr = msg->next_ptr;
      delete msg;
   }
}

bool QMsgQueue::push( QMsg* msg)
{
   if( msg == NULL)
   {
      semaphore.release();
      return false;
   }
   msg->resetWrittenSize();
   msg->next_ptr = NULL;

   QMutexLocker lock( &mutex);

   if( count == 0) firstPtr = msg;
   else lastPtr->next_ptr = msg;
   lastPtr = msg;

   semaphore.release();
   count++;

AFINFA("Msg* MsgQueue::push(): msg=%p, count=%d\n", msg, count);
   return true;
}

QMsg* QMsgQueue::pop( bool block)
{
   QMsg* msg = NULL;

   bool semresult = true;
   if( block)        semaphore.acquire();
   else semresult =  semaphore.tryAcquire();
   if( semresult == false ) return NULL;

   QMutexLocker lock( &mutex);
   if((count > 0) && (firstPtr != NULL))
   {
      msg = firstPtr;
      firstPtr = msg->next_ptr;
      msg->next_ptr = NULL;
      count--;
   }

AFINFA("Msg* MsgQueue::pop(): msg=%p, count=%d\n", msg, count);
   return msg;
}
