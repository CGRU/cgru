/******************************************************************************/
/*                                                                            */
/*    Copyright (c)The 3Delight Developers.                                   */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

#ifndef __dlRWLock_h
#define __dlRWLock_h

/*
	DlRWLock

	This class implements a 'reader/writer' lock where several threads can read
	at once but only a single thread can write.

	On unix we use the native pthread stuff but on win32 we have to roll our
	own. This means performance is not so great on win32 (as always) so don't
	abuse this class.
*/

class DlRWLock
{
public:
	DlRWLock();
	~DlRWLock();

	void ReadLock();
	void ReadUnlock();

	void WriteLock();
	void WriteUnlock();

private:
	void *m_data;
};

#endif // __dlRWLock_h
