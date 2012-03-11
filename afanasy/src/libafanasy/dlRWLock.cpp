/******************************************************************************/
/*                                                                            */
/*    Copyright (c)The 3Delight Developers.                                   */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

#include "dlRWLock.h"

/* patch so we can have rwlock with gcc 2.95.3 */
#if (__GNUC__ == 2)
#define _GNU_SOURCE
#endif

#include <assert.h>

#ifdef _WIN32

#include "dlMutex.h"

#include <windows.h>

struct RWLData
{
	/* This mutex is used to avoid starvation of writers by readers. */
	DlMutex m_new_reader;

	/* This event is used as a mutex and is held by all readers collectively or
	   by each writer. It's not a simple DlMutex because win32 docs suggest that
	   mutex objects (which critical sections use) can't be locked by a thread
	   and released by another thread. It seems to work anyway but I'll play
	   safe and use the event. */
	//DlMutex m_in_use;
	HANDLE m_in_use;

	/* The number of active readers; used to lock/unlock m_in_use. */
	unsigned m_reader_count;

	/* To change m_reader_count atomically. (TODO: replace by cmpxchg) */
	DlMutex m_count_mutex;
};

DlRWLock::DlRWLock()
{
	RWLData *data = new RWLData;
	data->m_reader_count = 0;
	data->m_in_use = CreateEvent(0x0, false, true, 0x0);

	m_data = data;
	
}

DlRWLock::~DlRWLock()
{
	RWLData *data = (RWLData*) m_data;

	assert(data->m_reader_count == 0);
	CloseHandle(data->m_in_use);

	delete data;
}

void DlRWLock::ReadLock()
{
	RWLData *data = (RWLData*) m_data;

	/* This is so we'll block if a writer is waiting. */
	data->m_new_reader.Lock();
	data->m_new_reader.Unlock();

	/* Increment reader count and grab the main lock if we're the first. */
	data->m_count_mutex.Lock();

	if (data->m_reader_count++ == 0)
	{
		//data->m_in_use.Lock();
		WaitForSingleObject(data->m_in_use, INFINITE);
	}

	data->m_count_mutex.Unlock();
}

void DlRWLock::ReadUnlock()
{
	RWLData *data = (RWLData*) m_data;

	/* Decrement reader count and release the main lock if we're the last. */
	data->m_count_mutex.Lock();

	if (--data->m_reader_count == 0)
	{
		//data->m_in_use.Unlock();
		SetEvent(data->m_in_use);
	}

	data->m_count_mutex.Unlock();
}

void DlRWLock::WriteLock()
{
	RWLData *data = (RWLData*) m_data;

	/* Prevent new readers to avoid starvation. */
	data->m_new_reader.Lock();

	/* Mutual exclusion with all readers and other writers. */
	//data->m_in_use.Lock();
	WaitForSingleObject(data->m_in_use, INFINITE);

	/* No need to hold this any longer; everyone will stall on m_in_use. */
	data->m_new_reader.Unlock();
}

void DlRWLock::WriteUnlock()
{
	RWLData *data = (RWLData*) m_data;

	/* Release everything. */
	//data->m_in_use.Unlock();
	SetEvent(data->m_in_use);
}

#else

#include <pthread.h>

DlRWLock::DlRWLock()
{
	pthread_rwlock_t *data = new pthread_rwlock_t;

	pthread_rwlock_init(data, 0x0);

	m_data = data;
}

DlRWLock::~DlRWLock()
{
	pthread_rwlock_t *data = (pthread_rwlock_t*) m_data;

	pthread_rwlock_destroy(data);

	delete data;
}

void DlRWLock::ReadLock()
{
	pthread_rwlock_t *data = (pthread_rwlock_t*) m_data;

	pthread_rwlock_rdlock(data);
}

void DlRWLock::ReadUnlock()
{
	pthread_rwlock_t *data = (pthread_rwlock_t*) m_data;

	pthread_rwlock_unlock(data);
}

void DlRWLock::WriteLock()
{
	pthread_rwlock_t *data = (pthread_rwlock_t*) m_data;

	pthread_rwlock_wrlock(data);
}

void DlRWLock::WriteUnlock()
{
	pthread_rwlock_t *data = (pthread_rwlock_t*) m_data;

	pthread_rwlock_unlock(data);
}

#endif
