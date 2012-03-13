#pragma once

#include "afqueue.h"
#include "msgaf.h"

/// Simple FIFO message queue
class MsgQueue : public AfQueue
{
public:
   MsgQueue( const QString & QueueName);
   ~MsgQueue();

/// Return first message from queue. BLOCKING FUNCTION if \c block==true .
   inline MsgAf* popMsg( bool block = true) { return (MsgAf*)pop( block);}

/// Push message to queue back.
   inline bool pushMsg( MsgAf* msg) { if(msg) msg->resetWrittenSize(); return push( msg);}

protected:
   void processItem( AfQueueItem* item) const;

private:
   void send( const MsgAf * msg, const af::Address * address) const;
};
