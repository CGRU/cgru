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
		SSReadOk,
		SSProcessing,
		SSWriting,
		SSWrittenOk,
		SSClosing,
		SSClosed,
	};

	inline int  getState() const { return m_state;}
	inline int  getSFD()   const { return m_sfd;}
	inline bool isZombie() const { return m_zombie;}

	bool readSocket();
	bool processMsg( ThreadArgs * i_args);
	void processRun( ThreadArgs * i_args);
	void writeSocket();
	void closeSocket();

private:
	int m_state;

	int m_sfd;
	struct sockaddr_storage * m_sas;

	af::Msg * m_msg_req;
	af::Msg * m_msg_ans;

	Profiler * m_profiler;

	bool m_zombie;
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

private:
	static SocketsProcessing * ms_this;

	std::list<SocketItem*> m_sockets;
//	DlRWLock m_sockets_lock;

	SocketQueue * m_queue_io;
	SocketQueue * m_queue_proc;
	SocketQueue * m_queue_run;

	std::vector<DlThread*> m_threads_io;
	static void ThreadFuncIO( void * i_args);
	void doIO();

	std::vector<DlThread*> m_threads_proc;
	static void ThreadFuncProc( void * i_args);
	void doProc();

	ThreadArgs * m_threadargs;
};

