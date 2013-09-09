/******************************************************************************/
/*                                                                            */
/*    Copyright (c)The 3Delight Developers.                                   */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

#include "dlThreadLocalStorage.h"

#include "dlCompileAssert.h"

#include <assert.h>

#ifdef _WIN32

#include <windows.h>

DlThreadLocalStorage::DlThreadLocalStorage()
{
	compile_assert(sizeof(m_id) >= sizeof(DWORD));
	
	DWORD idx = TlsAlloc();
	m_id = (unsigned) idx;
	
	assert(idx != TLS_OUT_OF_INDEXES);
}

DlThreadLocalStorage::~DlThreadLocalStorage()
{
	bool ret = TlsFree((DWORD) m_id);
	assert(ret);
}

void* DlThreadLocalStorage::Get()
{
	return TlsGetValue((DWORD) m_id);
}

void DlThreadLocalStorage::Set(void *i_value)
{
	bool ret = TlsSetValue((DWORD) m_id, i_value);
	assert(ret);
}

#else

#include <pthread.h>

DlThreadLocalStorage::DlThreadLocalStorage()
{
	compile_assert(sizeof(m_id) >= sizeof(pthread_key_t));

	int ret = pthread_key_create((pthread_key_t*) &m_id, 0x0);
	
	assert(ret == 0);	
}

DlThreadLocalStorage::~DlThreadLocalStorage()
{
	int ret = pthread_key_delete(*(pthread_key_t*) &m_id);
	assert(ret == 0);
}

void* DlThreadLocalStorage::Get()
{
	return pthread_getspecific(*(pthread_key_t*) &m_id);
}

void DlThreadLocalStorage::Set(void *i_value)
{
	int ret = pthread_setspecific(*(pthread_key_t*) &m_id, i_value);
	assert(ret == 0);
}

#endif
