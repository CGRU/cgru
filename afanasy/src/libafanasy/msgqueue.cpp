#include "msgqueue.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

MsgQueue::MsgQueue( const std::string & QueueName):
	AfQueue( QueueName, e_no_thread),
	m_verbose( VerboseOn)
{
}

MsgQueue::~MsgQueue()
{
}

