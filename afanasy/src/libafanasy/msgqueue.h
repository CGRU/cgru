#pragma once

#include "afqueue.h"
#include "msg.h"

namespace af
{

/// Simple FIFO message queue
class MsgQueue : public AfQueue
{
public:
    MsgQueue( const std::string & QueueName, StartTread i_start_thread);
    virtual ~MsgQueue();

    /// Return first message from queue. BLOCKING FUNCTION if \c block==AfQueue::e_wait.
    inline Msg* popMsg( WaitMode i_block ) { return (Msg*)pop(i_block);}

    /// Push message to queue back.
    inline bool pushMsg( Msg* msg) { if(msg) msg->resetWrittenSize(); return push( msg);}

    /// Answers and not sent message will be pushed in this queue if needed
    inline void setReturnQueue( MsgQueue * i_returnQueue) { m_returnQueue = i_returnQueue;}

    /// Messages that failed to send will be pushed in return queue
    inline void returnNotSended( bool i_value = true ) { m_returnNotSended = i_value;}

    inline void setVerboseMode( VerboseMode i_verbose) { m_verbose = i_verbose;}

protected:
    void processItem( AfQueueItem* item);

private:
    MsgQueue * m_returnQueue;
    bool m_returnNotSended;

    VerboseMode m_verbose;
};

} // namespace af
