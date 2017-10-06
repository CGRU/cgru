#include "socketsprocessing.h"

//######################################################################################################
//
//    TIME-WAIT:
//
// It is a special socket state, needed to ensure that all packages will not be lost.
// If server calls close() function first, its socket will fall into this state.
// To ensure that the connection last package is processed, it will wait:
//
//    TIME-WAIT = 2 * MSL (Maximum Segment Lifetime)
//
// This is the reason  why server should not call close() first.
// On a big amount of clients (1000), application can reach 2^16 ports limit.
// Here we wait for about 2sec to client to close socket first.
// To check socket connected state we just try to write in it.
// SIGPIPE is ignored in main.cpp
//
//
// To check sockets state you can:
//
// netstat -nat | grep 51000 | wc -l
// netstat -nat | egrep ':51000.*:.*TIME_WAIT' | wc -l
// ss -tan state time-wait | wc -l
// ss -tan 'sport = :51000' | awk '{print $(NF)" "$(NF-1)}' | sed 's/:[^ ]*//g' | sort | uniq -c
//
//######################################################################################################

#include "../libafanasy/common/dlThread.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "profiler.h"

#ifdef WINNT
#define MSG_DONTWAIT 0
#else
#define closesocket close
#endif

#ifdef LINUX
#include <sys/epoll.h>
#include <fcntl.h>
#endif

extern bool AFRunning;

#define AFOUTPUT
//#undef AFOUTPUT
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

	#ifdef LINUX
	m_epoll_added( false),
	m_bytes_read( 0),
	m_header_reading_finished( false),
	m_reading_finished( false),

	m_write_buffer( NULL),
	m_write_size(0),
	m_bytes_written(0),
	#endif // LINUX

	m_zombie(false)
{
	m_msg_req = new af::Msg( m_sas);

	m_profiler = new Profiler();

	af::setSocketOptions( m_sfd);

	#ifdef LINUX
	if( SocketsProcessing::UsingEpoll())
	{
		// Set descriptor non-blocking:
		int flags;
		flags = fcntl( m_sfd, F_GETFL);
		if( flags == -1 )
			AF_ERR << "fcntl: F_GETFL: " << strerror(errno);

		if( fcntl( m_sfd, F_SETFL, flags | O_NONBLOCK) == -1 )
			AF_ERR << "fcntl: F_SETFL: " << strerror(errno);
	}
	#endif // LINUX
}

SocketItem::~SocketItem()
{
	if( SSClosed != m_state )
	{
		if( AFRunning )
			AF_ERR << "Closing SocketItem socket in dtor: " << this;
		closeSocket();
	}

	delete m_sas;

	if( m_msg_req ) delete m_msg_req;
	if( m_msg_ans ) delete m_msg_ans;

	#ifdef LINUX
	if( m_write_buffer )
		delete [] m_write_buffer;
	#endif // LINUX

	// Delete profiler. 
	// If it was collected, pointer will be == NULL
	if( m_profiler )
		delete m_profiler;
}

bool SocketItem::readMsg()
{
	#ifdef LINUX
	if( SocketsProcessing::UsingEpoll())
		return readData();
	#endif

	// Read message data from socket
	if( false == af::msgread( m_sfd, m_msg_req))
	{
		// There was some error reading message:
		closeSocket();
		return false;
	}

	m_state = SSProcessing;
	return true;
}

void SocketItem::generateInfoStream( std::ostringstream & o_str, bool i_full) const
{
	af::sockAddrToStr( o_str, m_sas);
	o_str << " SFD:" << m_sfd << " S:";
	switch( m_state)
	{
		case SSReading:    o_str << "Reading";    break;
		case SSProcessing: o_str << "Processing"; break;
		case SSWriting:    o_str << "Writing";    break;
		case SSWaiting:    o_str << "SWaiting";   break;
		case SSClosed:     o_str << "Closed";     break;
		default:           o_str << "UNKNOWN";
	}
	if( m_zombie  ) o_str << " - ZOMBIE";
	if( m_msg_req ) o_str << " REQ: " << m_msg_req;
	if( m_msg_ans ) o_str << " ANS: " << m_msg_ans;
}

bool SocketItem::processMsg( ThreadArgs * i_args)
{
	// Return TRUE means ready to answer.
	if( m_state != SSProcessing )
	{
		AF_ERR << "A try to process invalid socket item state: " << this;
		return true;
	}

	m_profiler->processingStarted();

	if( m_msg_req->type() == af::Msg::THTTPGET )
	{
		m_msg_ans = httpGet( m_msg_req);
		m_profiler->processingFinished();
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
		// So no answer is ready.
		return false;
	}

	m_profiler->processingFinished();

	// Return TRUE means ready to answer.
	return true;
}

void SocketItem::processRun( ThreadArgs * i_args)
{
	m_msg_ans = threadRunCycleCase( i_args, m_msg_req);
	// Even if there is no answer, we should not close socket in RUN thread, as it can be a blocking operation.
	// It can block because we prefer to wait client closes socket first, to prevent TIME_WAIT socket state.

	m_profiler->processingFinished();
}

void SocketItem::writeMsg()
{
	if( m_state == SSWriting )
	{
		AF_ERR << "SocketItem::writeMsg: Repeated call on: " << this;
		return;
	}

	m_state = SSWriting;

	if( NULL == m_msg_ans )
	{
		// No answer exist means no answer needed.
		// For example on browser close (monitor deregister) it will not wait any answer
		waitClose();
		return;
	}

	// Set HTTP message type.
	// On writing header will be send first for web browsers.
	if( m_msg_req->type() == af::Msg::THTTP )
		m_msg_ans->setTypeHTTP();
	else if(( m_msg_req->type() == af::Msg::TJSONBIN ) && ( m_msg_ans->type() == af::Msg::TJSON ))
		m_msg_ans->setJSONBIN();

	#ifdef LINUX
	if( SocketsProcessing::UsingEpoll())
	{
		writeData();
		return;
	}
	#endif

	// Write response message back to client socket
	if( false == af::msgwrite( m_sfd, m_msg_ans))
	{
		AF_ERR << "Can't write answer back to client: " << this;
		af::printAddress( m_sas);
		m_msg_req->stdOutData();
		m_msg_ans->stdOutData();
		closeSocket();
		return;
	}

	waitClose();
}

#ifdef LINUX
bool SocketItem::processIO( int i_events)
{
	if( m_zombie )
	{
		AF_ERR << "EPOLL event on ZOMBIE socket item: " << this;
		return false;
	}

	if( i_events & EPOLLERR )
	{
		if( m_state != SSWaiting )
			AF_ERR << "EPOLLERR: " << af::sockAddrToStr( m_sas) << ": " << this;
		if( m_state != SSClosed )
			closeSocket();
		return false;
	}

	if( i_events & EPOLLHUP )
	{
		if( m_state != SSWaiting )
			AF_ERR << "EPOLLHUP: " << af::sockAddrToStr( m_sas) << ": " << this;
		if( m_state != SSClosed )
			closeSocket();
		return false;
	}

	switch( m_state )
	{
	case SSReading:
		return readData();
	case SSWriting:
		writeData();
		break;
	case SSWaiting:
		break;
	case SSClosed:
		// happens when several events are emitted for a given fd
		// and that a previous one made the fd close
		AF_WARN << "EPOLL event on a closed socket item: " << this;
		break;
	case SSProcessing:
		// This is not an error or even a warning.
		// Event can be generated after all data was read from previous event.
		break;
	default:
		AF_ERR << "EPOLL event on unknown socket item state: " << this;
	}

	return false;
}

bool SocketItem::readData()
{
	if( m_reading_finished )
	{
		// This is not an error/warning.
		// Event can happen if we read all data while adding this socket (from  accept thread),
		// But event was already triggered.
		return false;
	}

	if( false == m_header_reading_finished )
	{
		int size = read( m_sfd, m_msg_req->buffer() + m_bytes_read, af::Msg::SizeBuffer - m_bytes_read);
		if( size < 1 )
		{
			if( errno != EAGAIN )
			{
				AF_WARN << "Socket reading error: " << af::sockAddrToStr( m_sas) << ": " << this;
				closeSocket();
			}
			return false;
		}

		m_bytes_read += size;

		if( m_bytes_read < af::Msg::SizeHeader )
		{
			// Just continue reading header.
			return false;
		}

		m_header_reading_finished = true;

		int header_offset = af::processHeader( m_msg_req, m_bytes_read);
		if( header_offset < 0 )
		{
			// Negative offset means an invalid header.
			// This connection and its messages are not needed any more.
			closeSocket();
			return false;
		}

		if( m_msg_req->type() < af::Msg::TDATA )
		{
			// This message contains no data, all info is in its header.
			m_reading_finished = true;
		}
		else
		{
			m_bytes_read -= header_offset;

			if( m_bytes_read >= m_msg_req->dataLen())
			{
				// And header and data was read at the same operation.
				m_reading_finished = true;
			}
		}
	}

	if(( m_header_reading_finished ) && ( false == m_reading_finished ) && ( m_msg_req->type() >= af::Msg::TDATA ))
	{
		// Header was read, but message contains data which is not yet read:
		int toread = m_msg_req->dataLen() - m_bytes_read;
		int size = read( m_sfd, m_msg_req->buffer() + af::Msg::SizeHeader + m_bytes_read, toread);
		if( size < 1 )
		{	if( errno != EAGAIN )
			{
				AF_WARN << "Socket reading error: " << af::sockAddrToStr( m_sas) << ": " << this;
				closeSocket();
			}
			return false;
		}
		m_bytes_read += size;

		if( m_bytes_read >= m_msg_req->dataLen())
		{
			// All needed data was read.
			m_reading_finished = true;
		}
	}

	if( m_reading_finished )
	{
		// TRUE means that reading is successfully finished.
		// And this item will be pushed in the processing queue.
		m_state = SSProcessing;
		return true;
	}

	return false;
}

void SocketItem::writeData()
{
	if( NULL == m_msg_ans )
	{
		AF_ERR << "SocketItem::writeData(): The answer is NULL: " << this;
		return;
	}

	if( NULL ==  m_write_buffer )
	{
		// This is the first writing call.
		// We should allocate and fill in write buffer.
		int header_len = 0;
		char * header_buf = af::msgMakeWriteHeader( m_msg_ans );
		if( header_buf )
			header_len = strlen( header_buf);

		m_write_size = header_len + m_msg_ans->writeSize() - m_msg_ans->getHeaderOffset();
		m_write_buffer = new char[m_write_size];
		if( header_buf )
		{
			memcpy( m_write_buffer, header_buf, header_len);
			delete [] header_buf;
		}
		memcpy( m_write_buffer + header_len, m_msg_ans->buffer() + m_msg_ans->getHeaderOffset(), m_msg_ans->writeSize() - m_msg_ans->getHeaderOffset());
	}

	if( m_bytes_written >= m_write_size )
	{
		AF_WARN << "SocketItem::writeData(): m_bytes_written >= m_write_size ( " << m_bytes_written << " >= " << m_write_size << " ): " << this;
		return;
	}

	int bytes;
	#ifdef WINNT
	bytes = send( m_sfd, m_write_buffer + m_bytes_written, m_write_size - m_bytes_written, 0);
	#else
	bytes = write(m_sfd, m_write_buffer + m_bytes_written, m_write_size - m_bytes_written);
	#endif

	if( bytes >= 0 )
	{
		m_bytes_written += bytes;

		if( m_bytes_written >= m_write_size )
		{
			waitClose();
		}

		return;
	}

	switch( errno )
	{
	case EAGAIN:
		return;
	default:
		AF_ERR << "Socket writing error: " << af::sockAddrToStr( m_sas) << ": " << this;
	}

	closeSocket();

	return;
}
#endif // LINUX

void SocketItem::waitClose()
{
	// This function needed to wait for client closes socket first.
	// It is needed to prevent TIME-WAIT socket state on server side.
	// See this file top comments for details.

	if( SSClosed == m_state )
	{
		AF_WARN << "Asking to wait for close already closed SocketItem: " << this;
		return;
	}

	if( m_msg_ans == NULL )
	{
		// We will not wait client close if there is no answer.
		// There is no answer only in special cases.
		// For now there is only one "normal" case with no answer needed - web browser page close.
		// There is no way to ask browser to wait something when user closes page.
		closeSocket();
		return;
	}

	if(( false == af::Environment::getServerHTTPWaitClose()) &&
			(( m_msg_ans->type() == af::Msg::THTTP    ) ||
			(  m_msg_ans->type() == af::Msg::THTTPGET )))
	{
		// Web browsers do not prefer to close socket connection first.
		// To ask browser to close connection first we write a special HTTP header:
		// Connection: close
		// If your browser ignores this header, you can configure environment to not to wait it.
		closeSocket();
		return;
	}

	if( SSWaiting == m_state )
	{
		AF_WARN << "Asking to wait for close already waiting SocketItem: " << this;
		return;
	}

	m_state = SSWaiting;
	m_wait_time = time( NULL);

	#ifdef WINNT
	// Set socket non-blocking on Windows:
	u_long iMode = 1;
	int iResult = ioctlsocket( m_sfd, FIONBIO, &iMode);
	if (iResult != NO_ERROR)
		AF_ERR << "ioctlsocket failed with error: " << iResult;
	#endif
}

void SocketItem::checkClosed()
{
	if( SSWaiting != m_state )
	{
		AF_WARN << "Checking for close not waiting for close SocketItem: " << this;
		return;
	}

	static const char buf[] = "AFCLOSE";
	int r = send( m_sfd, buf, 1, MSG_DONTWAIT);
//AF_DEBUG << "r=" << r << " : " << this;
	if( r == -1 )
	{
		if( errno != EAGAIN )
		{
			closeSocket();
			return;
		}
	}

	if( time(NULL) - m_wait_time > 2 )
	{
		AF_WARN << "Client has NOT closed socket first: " << this;
		closeSocket();
	}
}

void SocketItem::closeSocket()
{
	if( SSClosed == m_state )
	{
		AF_WARN << "Asking to close already closed SocketItem: " << this;
		return;
	}

	m_state = SSClosed;

	#ifdef LINUX
	if( false == SocketsProcessing::UsingEpoll())
		SocketsProcessing::EpollDel( m_sfd);
	#endif // LINUX

	closesocket( m_sfd);

	Profiler::Collect( m_profiler);
	// Now Profiler takes ownership of this pointer in Collect().
	// We should set member to NULL to not to delete it in dtor.
	m_profiler = NULL;

	m_zombie = true;
}

#ifdef LINUX
bool SocketsProcessing::ms_epoll_enabled = false;
#endif
SocketsProcessing * SocketsProcessing::ms_this = NULL;
SocketsProcessing::SocketsProcessing( ThreadArgs * i_args):
	m_threadargs( i_args)
{
	#ifdef LINUX
	ms_epoll_enabled = af::Environment::getServerLinuxEpoll();
	#endif

	ms_this = this;
	m_threadargs->socketsProcessing = this;

	m_queue_io   = new SocketQueue("SocketsIO");
	m_queue_proc = new SocketQueue("SocketsProcess");
	m_queue_run  = new SocketQueue("SocketsRun");

	// Raising processing connections threads:
	AF_LOG << "Raising " << af::Environment::getServerSocketsProcessingThreadsNum() << " threads to process incoming connections...";
	for( int i = 0; i < af::Environment::getServerSocketsProcessingThreadsNum(); i++)
	{
		DlThread * t = new DlThread();
		// Set thread stack size if it is configured:
		if( af::Environment::getServerSocketsProcessingThreadsStack() > 0 )
		{
			t->SetStackSize( af::Environment::getServerSocketsProcessingThreadsStack());

			if( m_threads_proc.size() == 0 )
				AF_LOG << "Processing thread size is set to " << af::Environment::getServerSocketsProcessingThreadsStack();
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

	#ifdef LINUX
	if( UsingEpoll())
		initEpoll();
	else
	#endif
		initThreadingIO();
}

void SocketsProcessing::initThreadingIO()
{
	// Raising read/write sockets threads:
	AF_LOG << "Raising " << af::Environment::getServerSocketsReadWriteThreadsNum() << " threads to read/write sockets...";
	for( int i = 0; i < af::Environment::getServerSocketsReadWriteThreadsNum(); i++)
	{
		DlThread * t = new DlThread();
		// Set thread stack size if it is configured:
		if( af::Environment::getServerSocketsReadWriteThreadsStack() > 0 )
		{
			t->SetStackSize( af::Environment::getServerSocketsReadWriteThreadsStack());

			if( m_threads_io.size() == 0 )
				AF_LOG << "Processing thread size is set to " << af::Environment::getServerSocketsReadWriteThreadsStack();
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
}

SocketsProcessing::~SocketsProcessing()
{
	// Make queues to emit NULLs to awake waiting threads
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

	#ifdef LINUX
	if( UsingEpoll())
	{
		AF_LOG << "Finishing EPOLL...";
		close( m_epoll_fd);
		m_epoll_thread->Join();
		delete m_epoll_thread;
	}
	#endif

	AF_LOG << "Deleting remaining " << m_sockets.size() << " socket items...";
	std::list<SocketItem*>::iterator it = m_sockets.begin();
	for( ;  it != m_sockets.end(); it++)
	{
		delete *it;
	}

	AF_LOG << "Clearing profiling data...";
	Profiler::Destroy();

	delete m_queue_run;
	delete m_queue_proc;
	delete m_queue_io;

	AF_LOG << "Deleting sockets processing.";
}

void SocketsProcessing::acceptSocket( int i_sfd, sockaddr_storage * i_sas)
{
	SocketItem * si = new SocketItem( i_sfd, i_sas);

	#ifdef LINUX
	if( UsingEpoll())
	{
		m_queue_io->pushSI( si);
		return;
	}
	#endif


	// Process waiting sockets:
	std::list<SocketItem*>::iterator it = m_sockets.begin();
	while( it != m_sockets.end())
	{
		// Check waiting items:
		if((*it)->getState() == SocketItem::SSWaiting )
		{
			(*it)->checkClosed();
		}

		// Free zombies:
		if((*it)->isZombie())
		{
			SocketItem * zombie = *it;
			it = m_sockets.erase( it);
			delete zombie;
			continue;
		}

		it++;
	}


	m_sockets.push_back( si);

	m_queue_io->pushSI( si);
}

void SocketsProcessing::ThreadFuncIO( void * i_args)
{
	while( AFRunning )
	{
		ms_this->doBlockingIO();
	}
}

void SocketsProcessing::doBlockingIO()
{
	SocketItem * si = m_queue_io->popSI( af::AfQueue::e_wait);

	if( NULL == si ) return;

	switch( si->getState())
	{
	case SocketItem::SSReading:
		if( si->readMsg())
			m_queue_proc->pushSI( si);
		break;
	case SocketItem::SSProcessing:
		si->writeMsg();
		break;
	default:
		AF_ERR << "BlockingIO: Invalid socket item state: " << si;
	}
}

void SocketsProcessing::ThreadFuncProc( void * i_args)
{
	while( AFRunning )
		ms_this->doProc();
}

void SocketsProcessing::doProc()
{
	SocketItem * si = m_queue_proc->popSI( af::AfQueue::e_wait);
	if( NULL == si )
		return;

	if( si->processMsg( m_threadargs))
		m_queue_io->pushSI( si);
	else
		m_queue_run->pushSI( si);
}

void SocketsProcessing::processRun()
{
	SocketItem * si;
	while( ( si = m_queue_run->popSI( af::AfQueue::e_no_wait)) )
	{
		si->processRun( m_threadargs);
		m_queue_io->pushSI( si);
	}	
}
#ifdef LINUX
void SocketsProcessing::initEpoll()
{
	// Create EPOLL:
	m_epoll_fd = epoll_create(0xCAFE);
	if( m_epoll_fd == -1 )
	{
		AF_ERR << "epoll_create: " << strerror( errno);
		return;
	}

	// Start one thread for epoll_wait:
	m_epoll_thread = new DlThread();
	m_epoll_thread->Start( ThreadFuncEpoll, NULL );
}

void SocketsProcessing::epollAddSocket( SocketItem * i_si)
{
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLOUT;
	ev.events |= EPOLLET; // edge triggering
	ev.events |= EPOLLHUP | EPOLLERR;

	ev.data.ptr = static_cast<void*>(i_si);
	if( epoll_ctl( m_epoll_fd, EPOLL_CTL_ADD, i_si->getSFD(), &ev) == -1)
	{
		AF_ERR << "epoll_ctl: " << strerror(errno);
		return;
	}

	i_si->setEpollAdded();
}

void SocketsProcessing::ThreadFuncEpoll( void * i_args)
{
	AF_WARN << "Using non-blocking IO based on Linux EPOLL facility.";
	while( AFRunning )
		ms_this->doEpoll();
}

void SocketsProcessing::doEpoll()
{
	// Add incoming sockets, to read after accept, or to write after processing:
	SocketItem * si;
	while(( si = m_queue_io->popSI( af::AfQueue::e_no_wait)))
	{
		if( false == AFRunning )
			return;

		switch( si->getState())
		{
		case SocketItem::SSReading:
		{
			m_sockets.push_back( si);

			// If this is receiving, we start reading it because connections are edge
			// triggered so nothing would trigger for the already queued bytes.
			/*
			if( si->readMsg())
			{
				m_queue_proc->pushSI( si);
			}
			else
			*/
				epollAddSocket( si);

			break;
		}
		case SocketItem::SSProcessing:
		{
			if( false == si->isEpollAdded())
				epollAddSocket( si);
			si->writeMsg();
			break;
		}
		default:
			AF_ERR << "Got a new socket item with an invalid state: " << si;
		}
	}


	// EPOLL WAIT:
	static const int ep_max_events = 64;
	struct epoll_event events[ep_max_events];
	int nfds = epoll_wait( m_epoll_fd, events, ep_max_events, 128);
	if( nfds == -1 )
	{
		switch( errno)
		{
		case EINTR:
			AF_WARN << "epoll_wait: interrupted";
			return;
		default:
			AF_ERR << "epoll_wait: " << strerror( errno);
			AFRunning = false;
		}

		return;
	}
	for( int n = 0 ; n < nfds ; n++)
	{
		if( false == AFRunning )
			return;

		SocketItem * si = static_cast<SocketItem*>( events[n].data.ptr);

		if( si->processIO( events[n].events))
			m_queue_proc->pushSI( si);
	}


	// Process waiting sockets:
	std::list<SocketItem*>::iterator it = m_sockets.begin();
	while( it != m_sockets.end())
	{
		// Check waiting items:
		if((*it)->getState() == SocketItem::SSWaiting )
		{
			(*it)->checkClosed();
		}

		// Free zombies:
		if((*it)->isZombie())
		{
			SocketItem * zombie = *it;
			it = m_sockets.erase( it);
			delete zombie;
			continue;
		}

		it++;
	}
}

void SocketsProcessing::EpollDel( int i_sfd)
{
	epoll_ctl( ms_this->m_epoll_fd, EPOLL_CTL_DEL, i_sfd, NULL);
}
#endif // LINUX

