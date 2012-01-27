/******************************************************************************/
/*                                                                            */
/*    Copyright (c)The 3Delight Developers.                                   */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

#include "dlMutex.h"

#include "dlCompileAssert.h"

/* patch so we can have PTHREAD_MUTEX_RECURSIVE with gcc 2.95.3 */
#if (__GNUC__ == 2)
#define _GNU_SOURCE
#endif

#ifdef _WIN32
#	define _WIN32_WINNT 0x0400 /* for TryEnterCriticalSection */
#	include <windows.h>
#else
#	include <pthread.h>
#endif

#ifdef _WIN32
/* This typedef allows m_data casts to be kept out of the other ifdefs. */
typedef CRITICAL_SECTION pthread_mutex_t;
#endif

DlMutex::DlMutex()
{
	compile_assert(sizeof(m_data) >= sizeof(pthread_mutex_t));

	pthread_mutex_t *mutex = (pthread_mutex_t*) &m_data[0];

#ifdef _WIN32
	InitializeCriticalSection(mutex);
#else
	pthread_mutexattr_t attr;
	
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_init(mutex, &attr);

	pthread_mutexattr_destroy(&attr);
#endif
}

DlMutex::~DlMutex()
{
	pthread_mutex_t *mutex = (pthread_mutex_t*) &m_data[0];

#ifdef _WIN32
	DeleteCriticalSection(mutex);
#else
	pthread_mutex_destroy(mutex);
#endif
}

void DlMutex::Acquire()
{
	pthread_mutex_t *mutex = (pthread_mutex_t*) &m_data[0];

#ifdef _WIN32
	EnterCriticalSection(mutex);
#else
	pthread_mutex_lock(mutex);
#endif
}

bool DlMutex::TryAcquire()
{
	pthread_mutex_t *mutex = (pthread_mutex_t*) &m_data[0];

#ifdef _WIN32
	return 0 != TryEnterCriticalSection(mutex);
#else
	return 0 == pthread_mutex_trylock(mutex);
#endif
}

void DlMutex::Release()
{
	pthread_mutex_t *mutex = (pthread_mutex_t*) &m_data[0];

#ifdef _WIN32
	LeaveCriticalSection(mutex);
#else
	pthread_mutex_unlock(mutex);
#endif
}
