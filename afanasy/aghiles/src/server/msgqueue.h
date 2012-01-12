#pragma once

#include "afqueue.h"
#include "msgaf.h"

/// Simple FIFO message queue
class MsgQueue : public AfQueue
{
public:
   MsgQueue( const std::string & QueueName);
   virtual ~MsgQueue();

/// Return first message from queue. BLOCKING FUNCTION if \c block==AfQueue::e_wait.
   inline MsgAf* popMsg( WaitMode i_block ) { return (MsgAf*)pop(i_block);}

/// Push message to queue back.
   inline bool pushMsg( MsgAf* msg) { if(msg) msg->resetWrittenSize(); return push( msg);}

protected:
   void processItem( AfQueueItem* item);

private:
   void send( const af::Msg * msg, const af::Address & address) const;
};
