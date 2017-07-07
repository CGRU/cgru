#include "socketsprocessing.h"

#include "../libafanasy/common/dlThread.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "profiler.h"

extern bool AFRunning;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

af::Msg * httpGet( const af::Msg * i_msg);
af::Msg * threadProcessMsgCase( ThreadArgs * i_args, af::Msg * i_msg);
af::Msg * threadRunCycleCase( ThreadArgs * i_args, af::Msg * i_msg);

SocketItem::SocketItem( int i_sfd, sockaddr_storage * i_sas):
	m_state( SSReading),
	m_sfd( i_sfd),
	m_sas( i_sas),
	m_msg_req( NULL),
	m_msg_ans( NULL),
	m_zombie( false)
{
AF_DEBUG << m_sfd;
	m_profiler = new Profiler();

	af::setSocketOptions( m_sfd);
}

SocketItem::~SocketItem()
{
AF_DEBUG << m_sfd;
	if( SSClosed != m_state )
	{
		AF_ERR << "Closing SocketItem socket in dtor.";
		closeSocket();
	}

	delete m_sas;
	if( m_msg_req ) delete m_msg_req;
	if( m_msg_ans ) delete m_msg_ans;

	// Delete profiler if it was not collected.
	if( m_profiler ) delete m_profiler;
}

bool SocketItem::readSocket()
{
AF_DEBUG << m_sfd;
	m_msg_req = new af::Msg( m_sas);

	// Read message data from socket
	if( false == af::msgread( m_sfd, m_msg_req))
	{
		// There was some error reading message:
		closeSocket();
		return false;
	}

	return true;
}

bool SocketItem::processMsg( ThreadArgs * i_args)
{
AF_DEBUG << m_sfd;
	m_state = SSProcessing;
	m_profiler->processingStarted();

	if( m_msg_req->type() == af::Msg::THTTPGET )
	{
		m_msg_ans = httpGet( m_msg_req);
		m_profiler->processingFinished();
		m_state = SSWriting;
		return true;
	}
/*
	// Check message IP trust mask:
	if( false == m_msg_req->getAddress().matchIpMask())
	{
		// Authenticate message that does not match trust mask:
		if( false == Auth::process( m_msg_req, &m_msg_ans))
		{
			m_profiler->processingFinished();
			return;
		}
	}
*/

	m_msg_ans = threadProcessMsgCase( i_args, m_msg_req);
	
	if( m_msg_ans == NULL)
	{
		// No answer means that message was not processed.
		// And it will be pushed to run thread queue.
		return false;
	}

	if( m_msg_ans )
	{
		m_profiler->processingFinished();
		m_state = SSWriting;
	}

	return true;
}

void SocketItem::processRun( ThreadArgs * i_args)
{
	m_msg_ans = threadRunCycleCase( i_args, m_msg_req);
	// Even if there is no answer, we should not close socket in RUN thread, as it is a blocking opeartion.

	m_profiler->processingFinished();
	m_state = SSWriting;
}

void SocketItem::writeSocket()
{
AF_DEBUG << m_sfd;
	if( NULL == m_msg_ans )
	{
		// No answer exist means no answer needed.
		// For example on browser close (monitor deregister) it will not wait any answer
		closeSocket();
		return;
	}

	// Set HTTP message type.
	// On writing header will be send first for web browsers.
	if( m_msg_req->type() == af::Msg::THTTP )
		m_msg_ans->setTypeHTTP();
	else if(( m_msg_req->type() == af::Msg::TJSONBIN ) && ( m_msg_ans->type() == af::Msg::TJSON ))
		m_msg_ans->setJSONBIN();

	// Write response message back to client socket
	if( false == af::msgwrite( m_sfd, m_msg_ans))
	{
		AF_ERR << "Can't write answer back to client.";
		af::printAddress( m_sas);
		m_msg_req->stdOutData();
		m_msg_ans->stdOutData();
	}

	closeSocket();
}

void SocketItem::closeSocket()
{
AF_DEBUG << m_sfd;
	if( SSClosed == m_state )
	{
		AF_ERR << "Asking to close already closed SocketItem.";
		return;
	}

	int answer_type = -1;
	if( m_msg_ans )
		answer_type = m_msg_ans->type();

	af::socketDisconnect( m_sfd, answer_type);

	Profiler::Collect( m_profiler);
	// Now Profiler takes ownership of this pointer in Collect().
	// We should set member to NULL to not to delete it in dtor.
	m_profiler = NULL;

	m_state = SSClosed;

	m_zombie = true;
}

SocketsProcessing * SocketsProcessing::ms_this = NULL;
SocketsProcessing::SocketsProcessing( ThreadArgs * i_args):
	m_threadargs( i_args)
{
	ms_this = this;
	m_threadargs->socketsProcessing = this;

	m_queue_io   = new SocketQueue("SocketsIO");
	m_queue_proc = new SocketQueue("SocketsProcess");
	m_queue_run  = new SocketQueue("SocketsRun");

	// Raising icoming connections threads:
	AF_LOG << "Raising " << af::Environment::getServerSocketsProcessingThreadsNum() << " threads to read/write incoming connections...";
	for( int i = 0; i < af::Environment::getServerSocketsProcessingThreadsNum(); i++)
	{
		DlThread * t = new DlThread();
		// Set thread stack size if it is configured:
		if( af::Environment::getServerSocketsProcessingThreadsStack() > 0 )
		{
			t->SetStackSize( af::Environment::getServerSocketsProcessingThreadsStack());

			if( m_threads_io.size() == 0 )
				AF_LOG << "Processing thread size is set to " << af::Environment::getServerSocketsProcessingThreadsStack();
		}

		int retval = t->Start( ThreadFuncIO, NULL );
		switch( retval)
		{
			case 0:      m_threads_io.push_back(t); break;
			case EAGAIN: AF_ERR << "Insufficient resources to create another thread."; break;
			case EINVAL: AF_ERR << "Invalid thread settings."; break;
			case EPERM:  AF_ERR << "No permission to set the scheduling policy and parameters specified."; break;
			default:     AF_ERR << "Unknown error creating new thread.";
		}
	}
	AF_LOG << "Raised " << m_threads_io.size() << " threads.";


	// Raising processings connections threads:
	AF_LOG << "Raising " << af::Environment::getServerSocketsReadWriteThreadsNum() << " threads to process incomming connections...";
	for( int i = 0; i < af::Environment::getServerSocketsReadWriteThreadsNum(); i++)
	{
		DlThread * t = new DlThread();
		// Set thread stack size if it is configured:
		if( af::Environment::getServerSocketsReadWriteThreadsStack() > 0 )
		{
			t->SetStackSize( af::Environment::getServerSocketsReadWriteThreadsStack());

			if( m_threads_proc.size() == 0 )
				AF_LOG << "Processing thread size is set to " << af::Environment::getServerSocketsReadWriteThreadsStack();
		}

		int retval = t->Start( ThreadFuncProc, NULL );
		switch( retval)
		{
			case 0:      m_threads_proc.push_back(t); break;
			case EAGAIN: AF_ERR << "Insufficient resources to create another thread."; break;
			case EINVAL: AF_ERR << "Invalid thread settings."; break;
			case EPERM:  AF_ERR << "No permission to set the scheduling policy and parameters specified."; break;
			default:     AF_ERR << "Unknown error creating new thread.";
		}
	}
}

SocketsProcessing::~SocketsProcessing()
{
	AF_DEBUG;

	// Make queues to emit NULLs to awake wainting threads
	m_queue_run->releaseNull();
	for( int i = 0; i < m_threads_io.size(); i++)
		m_queue_io->releaseNull();
	for( int i = 0; i < m_threads_proc.size(); i++)
		m_queue_proc->releaseNull();

	AF_LOG << "Joining sockets processing threads...";
	for( int i = 0; i < m_threads_io.size(); i++)
		m_threads_io[i]->Join();
	for( int i = 0; i < m_threads_proc.size(); i++)
		m_threads_proc[i]->Join();

	for( int i = 0; i < m_threads_io.size(); i++)
		delete m_threads_io[i];
	for( int i = 0; i < m_threads_proc.size(); i++)
		delete m_threads_proc[i];

	AF_LOG << "Deleting remaining " << m_sockets.size() << " socket items...";
	std::list<SocketItem*>::iterator it = m_sockets.begin();
	for( ;  it != m_sockets.end(); it++)
	{
		delete *it;
	}

	delete m_queue_run;
	delete m_queue_proc;
	delete m_queue_io;

	AF_LOG << "Deleting sockets processing.";
}

void SocketsProcessing::acceptSocket( int i_sfd, sockaddr_storage * i_sas)
{
	SocketItem * si = new SocketItem( i_sfd, i_sas);

	m_queue_io->pushSI( si);

//	m_sockets_lock.WriteLock();

	m_sockets.push_back( si);

	std::list<SocketItem*>::iterator it = m_sockets.begin();
	for( ;  it != m_sockets.end(); it++)
	{
		if((*it)->isZombie())
		{
			SocketItem * zombie = *it;
			it = m_sockets.erase( it);
			delete zombie;
		}
	}

//	m_sockets_lock.WriteUnlock();

	AF_DEBUG << "Sockets count: " << m_sockets.size();
}

void SocketsProcessing::ThreadFuncIO( void * i_args)
{
	while( AFRunning )
	{
		ms_this->doIO();
	}
}

void SocketsProcessing::doIO()
{
	SocketItem * si = m_queue_io->popSI( af::AfQueue::e_wait);

	if( NULL == si ) return;

	switch( si->getState())
	{
	case SocketItem::SSReading:
		if( si->readSocket())
			m_queue_proc->pushSI( si);
		break;
	case SocketItem::SSWriting:
		si->writeSocket();
		break;
	default:
		AF_ERR << "Unknown socket item state: " << si->getState();
	}
}

void SocketsProcessing::ThreadFuncProc( void * i_args)
{
	while( AFRunning )
	{
		ms_this->doProc();
	}
}

void SocketsProcessing::doProc()
{
	SocketItem * si = m_queue_proc->popSI( af::AfQueue::e_wait);

	if( NULL == si ) return;

	if( si->processMsg( m_threadargs))
		m_queue_io->pushSI( si);
	else
		m_queue_run->pushSI( si);
}

void SocketsProcessing::processRun()
{
	SocketItem * si;
	while( (si = m_queue_run->popSI( af::AfQueue::e_no_wait)) )
	{
		si->processRun( m_threadargs);
		m_queue_io->pushSI( si);
	}	
}

