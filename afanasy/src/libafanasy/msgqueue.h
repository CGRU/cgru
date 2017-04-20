#pragma once

#include "afqueue.h"
#include "msg.h"

namespace af
{

/// Simple FIFO message queue
class MsgQueue : public AfQueue
{
public:
	MsgQueue( const std::string & QueueName);
	~MsgQueue();

	/// Return first message from queue. BLOCKING FUNCTION if \c block==AfQueue::e_wait.
	inline Msg* popMsg( WaitMode i_block ) { return (Msg*)pop(i_block);}

	/// Push message to queue back.
	inline bool pushMsg( Msg* msg) { if(msg) msg->resetWrittenSize(); return push( msg);}

	inline void setVerboseMode( VerboseMode i_verbose) { m_verbose = i_verbose;}

private:

	VerboseMode m_verbose;
};

} // namespace af
