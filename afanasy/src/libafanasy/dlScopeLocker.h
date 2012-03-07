/******************************************************************************/
/*                                                                            */
/*    Copyright (c)The 3Delight Developers.                                   */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

#ifndef __dlScopeLocker_h
#define __dlScopeLocker_h

/*
	DlScopeLocker
	
	A class to grab mutex and have it automatically unlocked when going out of
	scope. Useful when a function has several exit points or when the mutex may
	or may not have been locked when the end of the function is reached.
*/

template <typename MutexType>
class DlGenericScopeLocker
{
public:
	DlGenericScopeLocker() : m_mutex(0x0) {}

	DlGenericScopeLocker(MutexType *i_mutex)
	{
		Lock(i_mutex);
	}

	~DlGenericScopeLocker()
	{
		Unlock();
	}

	void Lock(MutexType *i_mutex)
	{
		m_mutex = i_mutex;
		m_mutex->Lock();
	}

	bool TryLock(MutexType *i_mutex)
	{
		if (i_mutex->TryLock())
		{
			m_mutex = i_mutex;
			return true;
		}

		return false;
	}
	
	void Unlock()
	{
		if (m_mutex)
			m_mutex->Unlock();
		
		m_mutex = 0x0;
	}

	/* Be careful as the single-thread implementation always returns true. */
	bool Locked() const { return m_mutex != 0x0; }

private:
	MutexType *m_mutex;
};

class DlMutex;
class DlTinyMutex;

typedef DlGenericScopeLocker<DlMutex> DlScopeLocker;
typedef DlGenericScopeLocker<DlTinyMutex> DlTinyScopeLocker;

#endif // __dlScopeLocker_h
