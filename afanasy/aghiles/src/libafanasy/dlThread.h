/******************************************************************************/
/*                                                                            */
/*    Copyright (c)The 3Delight Developers.                                   */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

#ifndef __dlThread_h
#define __dlThread_h

/*
	This class implements the basic thread abstraction. It's purpose is to hide
	OS-specific details.
	
	safety guide:
	
	- Stay away from cancellation if possible. Don't expect it to work anywhere
	  but in TestCancel() and Sleep().
	- Only Join() a thread once.
*/

class DlThread
{
	DlThread(const DlThread&);
	void operator=(const DlThread&);

public:
	DlThread();
	~DlThread();

	/*
		Sets the thread to be started in detached mode. This means it may not
		(and must not) be joined and the DlThread will delete itself when the
		thread function exits. Such a thread may also not use cancellation.
	*/
	void SetDetached();

	/* Start a new thread. */
	void Start(void (*i_thread_func)(void*), void *i_arg);
	
	/*
		Join
		
		Waits for the thread to end.
		
		- If the thread hasn't been started, this function returns right away.
		- If the thread has already been joined, it returns right away too.
		- Only one thread may try to join on another thread.
	*/
	void Join();
	
	/*
		Cancel
	
		Signal the thread to cancel.
		
		NOTES
		- Does NOT wait for actual cancellation to be complete. There is a 99%
		  chance that you'll want to call Join() right after Cancel().
	*/
	void Cancel();
	
	/*
		TestCancel()
	
		This is an arbitrary cancellation point.
		
		NOTES:
		- The only other one provided is Sleep().
	*/
	void TestCancel();
		
	/*
		Sleep
		
		Sleeps the current thread for a given amount of time.
		
		NOTES
		- This is a cancellation point.
	*/
	static void Sleep(unsigned i_seconds);
	
	/*
		Self
		
		Returns the DlThread instance for the calling thread.
	*/
	static DlThread* Self();
	
	/*
		GetNbProcessors
		
		return the number of processors on this machine.
	*/
	static unsigned GetNbProcessors();

private:
	/* Perform cleanup when the thread ends. */
	void Cleanup();
	
	/* Sleep implementation. */
	static void SleepCancel(void *i_thread);
	void SleepSelf(unsigned i_seconds);

	/* Thread startup functions. */
#ifdef _WIN32
	static unsigned __stdcall thread_routine(void *i_params);
#else
	static void* thread_routine(void *i_params);
#endif

private:
	/* This is opaque because it needs a lot of system specific types. */
	struct ThreadData;
	
	ThreadData *m_data;
};

#endif // __dlThread_h
