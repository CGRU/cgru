/******************************************************************************/
/*                                                                            */
/*    Copyright (c)The 3Delight Developers.                                   */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

#include "dlThread.h"

#include "dlMutex.h"
#include "dlScopeLocker.h"
#include "dlThreadLocalStorage.h"

#ifdef _WIN32
#	include <windows.h>
#	include <process.h>
#	include <signal.h>
#else
#	include <pthread.h>
#	include <unistd.h>
#endif

/* The following chunk are for GetNbProcessors. */
#ifdef LINUX
#include	<sys/sysinfo.h>
#endif

#ifdef IRIX
#	include <sys/types.h>
#	include <sys/sysmp.h>
#	include <sys/sysinfo.h>
#endif

#ifdef DARWIN
#	include <mach/mach.h>
#	include <mach/mach_host.h>
#	include <mach/host_info.h>
#endif

#include <assert.h>

#include <list>

/*
	This TLS slot is used to store a DlThread instance for each thread in the
	process. This is used by the Self() method.
*/
static DlThreadLocalStorage s_thread_instance;

/*
	List of all DlThread objects and mutex to protect it. This is needed for
	user TLS to be able to clear newly allocated slots in all threads. Each
	thread holds an iterator which points to its entry in this list.
*/
DlMutex s_all_threads_mutex;
std::list<DlThread*> s_all_threads;

#ifdef _WIN32
typedef void (__cdecl *windows_sig_handler_t)(int);
static const int propagated_signals[] = { SIGILL, SIGABRT, SIGFPE, SIGSEGV };
static const unsigned num_propagated_signals =
	sizeof(propagated_signals) / sizeof(propagated_signals[0]);
#endif

/*
	Private data for each thread instance. This isn't simply in the class
	because it uses system-specific data types.
*/
struct DlThread::ThreadData
{
	/* true when signaled with Cancel() */
	bool m_must_cancel;

	/* true if the thread is to be started in a detached state. */
	bool m_start_detached;

#ifdef _WIN32
	/* Event used on windows to interrupt Sleep() when cancelling. */
	HANDLE m_cancellation_event;

	/* Signal handlers in the parent thread. Must be propagated to child. */
	windows_sig_handler_t m_signal_handlers[num_propagated_signals];
#endif

	/* system thread handle */
#ifdef _WIN32
	HANDLE m_handle;
#else
	pthread_t m_handle;
#endif

	/* Thread's function and arguments. */
	void (*m_thread_func)(void*);
	void *m_thread_arg;

	/* Points to our entry in s_all_threads. */
	std::list<DlThread*>::iterator m_global_table_it;
};

/*
	DlThread constructor
	
	Creates an empty, uselss thread.
*/
DlThread::DlThread()
{
	m_data = new ThreadData;

	m_data->m_must_cancel = false;
	m_data->m_start_detached = false;
	m_data->m_handle = 0;

#ifdef _WIN32
	m_data->m_cancellation_event = CreateEvent(0x0, false, false, 0x0);
#endif

	DlScopeLocker global_table_l( &s_all_threads_mutex );
	m_data->m_global_table_it =
		s_all_threads.insert( s_all_threads.end(), this );
}

/*
	DlThread destructor
*/
DlThread::~DlThread()
{
	/* Must have been detached or joined. */
	assert(!m_data->m_handle);

#ifdef _WIN32
	CloseHandle(m_data->m_cancellation_event);
#endif

	DlScopeLocker global_table_l( &s_all_threads_mutex );
	s_all_threads.erase( m_data->m_global_table_it );

	delete m_data;
	m_data = 0x0;
}

/*
	SetDetached

	Sets the thread to start in detached mode. See .h for details.
*/
void DlThread::SetDetached()
{
	if( m_data->m_handle )
	{
		assert( false );
		return;
	}

	m_data->m_start_detached = true;
}

/*
	thread_routine
	Start
	
	These two handle starting an actual thread of execution associated with this
	class.
*/

#ifdef _WIN32
unsigned __stdcall DlThread::thread_routine(void *i_params)
{
#else
void* DlThread::thread_routine(void *i_params)
{
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0x0);
#endif
	DlThread *t = (DlThread*) i_params;

#ifdef _WIN32
	/* Set the same signal handlers as the parent. */
	for( unsigned i = 0; i < num_propagated_signals; ++i )
	{
		signal( propagated_signals[i], t->m_data->m_signal_handlers[i] );
	}
#endif

	s_thread_instance.Set(t);

	t->m_data->m_thread_func(t->m_data->m_thread_arg);
	
	t->Cleanup();

	/* If this is a detached thread, it is our job to delete the DlThread. */
	if( t->m_data->m_start_detached )
	{
		delete t;
	}

	return 0;
}

void DlThread::Start(void (*i_thread_func)(void*), void *i_arg)
{
	assert(!m_data->m_handle);

	m_data->m_thread_func = i_thread_func;
	m_data->m_thread_arg = i_arg;

	/* Read this right now as the DlThread (this) can be deleted as soon as we
	   start a detached thread. */
	bool detached = m_data->m_start_detached;

#ifdef _WIN32
	/* Signal handlers are per thread on windows so copy them to new thread. */
	for( unsigned i = 0; i < num_propagated_signals; ++i )
	{
		m_data->m_signal_handlers[i] = SIG_DFL;
		windows_sig_handler_t s = signal( propagated_signals[i], SIG_DFL );

		if( s == SIG_ERR )
			continue;

		m_data->m_signal_handlers[i] = s;
		signal( propagated_signals[i], s );
	}

	/* TODO : Should start suspended and then resume to ensure m_handle is set
	   if there is ever code which might have a use for it in the child thread.
	   Right now, there isn't so we don't bother.
	*/
	unsigned thread_id;
	
	HANDLE handle = (HANDLE) _beginthreadex(
		0x0,
		0,
		&thread_routine, this,
		0,
		&thread_id);

	if( detached )
	{
		CloseHandle( handle );
	}
	else
	{
		m_data->m_handle = handle;
	}
#else
	pthread_attr_t attr;
	pthread_attr_init( &attr );
	if( detached )
	{
		pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
	}

	pthread_t handle = 0;
	pthread_create( &handle, &attr, &thread_routine, this );

	pthread_attr_destroy( &attr );

	if( !detached )
	{
		m_data->m_handle = handle;
	}
#endif
}

/*
	Join
*/
void DlThread::Join()
{
	if (!m_data->m_handle)
	{
		/* Join() was called twice, before Start() or on detached thread. */
		assert(false);
		return;
	}

	assert( !m_data->m_start_detached );

#ifdef _WIN32
	WaitForSingleObject(m_data->m_handle, INFINITE);
	CloseHandle(m_data->m_handle);
#else
	pthread_join(m_data->m_handle, 0x0);
#endif

	m_data->m_handle = 0;
}

/*
	Cancel
	
	NOTES
	- Cancellation involves two mechanisms: the m_must_cancel flag and an
	  asynchronous method to cancel sleeping threads. On windows this is an
	  event and on linux this is pthread_cancel (cancellation is only enabled
	  while sleeping).
*/
void DlThread::Cancel()
{
	if( !m_data->m_handle || m_data->m_start_detached )
	{
		assert(false);
		return;
	}
	
	m_data->m_must_cancel = true;

#ifdef _WIN32
	SetEvent(m_data->m_cancellation_event);
#else
	pthread_cancel(m_data->m_handle);
#endif
}

/*
	TestCancel
	
	NOTES
	- This only tests for the m_must_cancel flag. Asynchronous cancellation
	  (done while sleeping) is checked for inside Sleep().
*/
void DlThread::TestCancel()
{
	assert( !m_data->m_start_detached );

	if (m_data->m_must_cancel)
	{
		Cleanup();
		
#ifdef _WIN32
		_endthreadex(0);
#else
		pthread_exit(0);
#endif
	}
}

/*
	Sleep
	
	Public interface to SleepSelf() to ensure one thread does not call Sleep()
	on another.
*/
void DlThread::Sleep(unsigned i_seconds)
{
	Self()->SleepSelf(i_seconds);
}

/*
	Self
*/
DlThread* DlThread::Self()
{
	DlThread *thread = (DlThread*) s_thread_instance.Get();

	if (!thread)
	{
		/*
			Thread was created without going through this class, either with
			system calls or because it's the main() thread. Create a
			corresponding instance in detached mode (will not join and will
			delete itself).
		*/
		thread = new DlThread();

		s_thread_instance.Set(thread);
	}
	
	return thread;
}

/*
	Cleanup
	
	Placeholder for thread cleanup. There's not much left now... this may go
	away soon.
*/
void DlThread::Cleanup()
{
	s_thread_instance.Set(0x0);
}

/*
	Handler for cancellation while sleeping.
*/
void DlThread::SleepCancel(void *i_thread)
{
	DlThread *thread = (DlThread*) i_thread;

	thread->Cleanup();
}

/*
	SleepSelf

	Just like the sleep() but with some hacks for thread cancellation.
	Initially tested in the netcache and moved here later.
	Later reworked to use the DlThread cancellation mechanisms.
*/
void DlThread::SleepSelf(unsigned i_seconds)
{
	TestCancel();

#if defined(_WIN32)
	/* This waits for cancellation, up to i_seconds seconds. */
	WaitForSingleObject( m_data->m_cancellation_event, i_seconds * 1000u );

#else

	/* Enable cancellation while sleeping. */
	
	pthread_cleanup_push(SleepCancel, this);

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0x0);

#ifdef DARWIN
	/* the cancellation point in sleep isn't implemented in OS X, see:

	http://lists.apple.com/archives/darwin-userlevel/2004/Mar/msg00016.html
	http://lists.apple.com/archives/darwin-kernel/2004/Jan/msg00032.html

	or google for 'pthread_cancel sleep OS X'. This means we have to check
	for cancellation explicitely. This will make the cancellation 'hang' a
	bit but there's no workaround until the OS X guys get their ass moving
	and implement decent threads.
	
	Update: It looks like the OS X people behind this are complete morons:
	http://lists.apple.com/archives/darwin-kernel/2007/Nov/msg00104.html

	Update(2): This got fixed at last in 10.6:
	http://lists.apple.com/archives/darwin-kernel/2009/Dec/msg00038.html
	We could remove this patch when we start targetting 10.6 and up.
	*/

	for (unsigned s = 0; s < i_seconds; ++s)
	{
		sleep(1);
		pthread_testcancel();
	}
#else
	sleep( i_seconds );

	/* It seems redhat 7 (libc 2.2.5) will not cancel properly when in the
	   sleep call. It will however get out of that call (probably due to
	   receiving a signal) so this simple test keeps us from hanging. */

	pthread_testcancel();
#endif

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0x0);

	pthread_cleanup_pop(0);
#endif

	TestCancel();
}

/*
	GetNbProcessors

	return the number of processors on this machine.
*/
unsigned DlThread::GetNbProcessors()
{
#ifdef _WIN32
	/* NOTE: This function will return the "logical" number of processors
	   on the system. So for example, the "hyperthreaded" processors might
	   give 2 here even if there is 1 processor onboard */

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
#endif

#ifdef LINUX
    return ::get_nprocs_conf();
#endif

#ifdef IRIX
    return ::sysmp(MP_NPROCS);
#endif

#ifdef DARWIN
	host_basic_info_data_t hostInfo;
	mach_msg_type_number_t infoCount;

	infoCount = HOST_BASIC_INFO_COUNT;
	host_info(
		mach_host_self(), HOST_BASIC_INFO, 
		(host_info_t)&hostInfo, &infoCount );

	return (unsigned int)(hostInfo.max_cpus);
#endif
}
