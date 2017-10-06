#pragma once

#include "../libafanasy/afqueue.h"
#include "../libafanasy/common/dlRWLock.h"
#include "../libafanasy/name_af.h"

#include "threadargs.h"

class DlThread;
class Profiler;

class SocketItem: public af::AfQueueItem
{
public:
	SocketItem( int i_sfd, sockaddr_storage * i_sas);
	~SocketItem();

	enum SocketState {
		SSReading,
		SSProcessing,
		SSWriting,
		SSWaiting, // We try to wait for client closes socket first
		SSClosed
	};

	void generateInfoStream( std::ostringstream & o_str, bool i_full = false) const;
	inline const std::string generateInfoString( bool i_full = false) const { std::ostringstream str; generateInfoStream( str, i_full); return str.str();}
	friend inline std::ostream & operator<<( std::ostream & o_str, const SocketItem * i_si) { o_str << i_si->generateInfoString(); return o_str; }

	inline int  getState() const { return m_state;}
	inline int  getSFD()   const { return m_sfd;}
	inline bool isZombie() const { return m_zombie;}

	bool readMsg();
	bool processMsg( ThreadArgs * i_args);
	void processRun( ThreadArgs * i_args);
	void writeMsg();
	void checkClosed();

	#ifdef LINUX
	// For non-blocking IO:
	bool processIO( int i_events);
	inline void setEpollAdded() { m_epoll_added = true; }
	inline bool isEpollAdded() const { return m_epoll_added; }
	#endif

private:
	void waitClose();
	void closeSocket();

private:
	int m_state;

	int m_sfd;
	struct sockaddr_storage * m_sas;

	af::Msg * m_msg_req;
	af::Msg * m_msg_ans;

	Profiler * m_profiler;

	time_t m_wait_time;
	bool m_zombie;

	#ifdef LINUX
	// For non-blocking IO:
	bool readData();
	int  m_bytes_read;
	bool m_header_reading_finished;
	bool m_reading_finished;
	bool m_epoll_added;

	void   writeData();
	char * m_write_buffer;
	int    m_write_size;
	int    m_bytes_written;
	#endif
};

class SocketQueue: public af::AfQueue
{
public:
	SocketQueue( std::string i_name): af::AfQueue( i_name, e_no_thread) {}
	~SocketQueue() {}
	inline void pushSI( SocketItem * i_si) { push( i_si);}
	inline SocketItem * popSI( WaitMode i_block ) { return (SocketItem*)(pop( i_block));}
};

class SocketsProcessing
{
public:
	SocketsProcessing( ThreadArgs * i_args);
	~SocketsProcessing();

	void acceptSocket( int i_sfd, sockaddr_storage * i_sas);

	void processRun();

	#ifdef LINUX
	inline static bool UsingEpoll() { return ms_epoll_enabled; }
	static void EpollDel( int i_sfd);
	#endif

private:
	static void ThreadFuncProc( void * i_args);
	void doProc();

	static SocketsProcessing * ms_this;

	ThreadArgs * m_threadargs;

	std::list<SocketItem*> m_sockets;

	SocketQueue * m_queue_io;
	SocketQueue * m_queue_proc;
	SocketQueue * m_queue_run;

	std::vector<DlThread*> m_threads_proc;
	std::vector<DlThread*> m_threads_io;


	// Blocking/Threading IO:
	static void ThreadFuncIO( void * i_args);
	void initThreadingIO();
	void doBlockingIO();


	#ifdef LINUX
	// Non-Blocking/EPOLL IO:
	static void ThreadFuncEpoll( void * i_args);
	void initEpoll();
	void doEpoll();
	void epollAddSocket( SocketItem * i_si);

	static bool ms_epoll_enabled;
	int m_epoll_fd;
	DlThread * m_epoll_thread;
	#endif
};
