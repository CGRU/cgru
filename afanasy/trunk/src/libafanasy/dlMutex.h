/******************************************************************************/
/*                                                                            */
/*    Copyright (c)The 3Delight Developers.                                   */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

#ifndef __dlMutex_h
#define __dlMutex_h

/*
	This class is a simple wrapper around system-specific mutex classes.
	The provided mutex is of the recursive type (can be locked several times by
	the same thread).
*/

class DlMutex
{
	DlMutex(const DlMutex&);
	void operator=(const DlMutex&);

	/* Needs access to pthread mutex. */
	friend class DlConditionVariable;

public:
	DlMutex();
	~DlMutex();
	
	void Acquire();
	bool TryAcquire();
	void Release();
	
	/* convenient synonyms */
	void Lock() { Acquire(); }
	bool TryLock() { return TryAcquire(); }
	void Unlock() { Release(); }

private:
	/* opaque data for system specific implementation */
	
#if defined(_WIN32)
#if defined(_WIN64)
	unsigned m_data[10];
#else
	unsigned m_data[6];
#endif
#elif defined(LINUX)
#if defined(__x86_64__) || defined(__powerpc64__)
	unsigned m_data[10];
#else
	unsigned m_data[6];
#endif
#elif defined(MACOSX)
	unsigned m_data[16];
#elif defined(IRIX)
	unsigned m_data[8];
#else
	unsigned m_data[1];
#endif

};

#endif // __dlMutex_h

