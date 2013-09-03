/******************************************************************************/
/*                                                                            */
/*    Copyright (c)The 3Delight Developers.                                   */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

#ifndef __dlThreadLocalStorage_h
#define __dlThreadLocalStorage_h

/*
	This class is an abstraction around thread-local storage.
	
	An instance of this class gives you access to a pointer which is stored in
	an different physical location for each thread.

	This class is a wrapper around system level TLS. Most of the time, you
	should use DlUserTLS instead as it will not run into resource limits.
*/

class DlThreadLocalStorage
{
	DlThreadLocalStorage(const DlThreadLocalStorage&);
	void operator=(const DlThreadLocalStorage&);

public:
	DlThreadLocalStorage();
	~DlThreadLocalStorage();

	void *Get();
	void Set(void *i_value);

private:
#if defined(MACOSX)
	void *m_id;
#else
	unsigned m_id;
#endif
};

#endif // __dlThreadLocalStorage_h
