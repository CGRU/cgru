#include "childprocess.h"

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

#if defined Q_OS_UNIX
#include <signal.h>
#else
#include <winbase.h>
/*
WINBASEAPI BOOL WINAPI SetInformationJobObject(HANDLE, JOBOBJECTINFOCLASS, LPVOID, DWORD);
in MinGW/include/winbase.h before this:
WINBASEAPI HANDLE WINAPI CreateJobObjectA(LPSECURITY_ATTRIBUTES,LPCSTR);
*/
#endif

ChildProcess::ChildProcess( QObject * parent):
   QProcess(parent)
{
}

ChildProcess::~ChildProcess()
{
}

void ChildProcess::setupChildProcess()
{
#if defined Q_OS_UNIX
   ::setsid();
   int nicenew = ::nice( af::Environment::getRenderNice());
   if( nicenew == -1)
      AFERRPE("nice");
#else
   PROCESS_INFORMATION *pi = pid();
   SetPriorityClass( pi->hProcess, BELOW_NORMAL_PRIORITY_CLASS);
   hJob = CreateJobObject( NULL, NULL);
   JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
   jeli.BasicLimitInformation.LimitFlags = 0x00002000;//JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
   if( SetInformationJobObject( hJob, JobObjectExtendedLimitInformation, &jeli, sizeof( jeli ) ) == 0)
      AFERROR("SetInformationJobObject failed.\n");
   if( AssignProcessToJobObject( hJob, pi->hProcess) == false)
      AFERROR("AssignProcessToJobObject failed.\n");
#endif
}

void ChildProcess::kill()
{
#if defined Q_OS_UNIX
   ::killpg(::getpgid(pid()), SIGKILL);
#else
   CloseHandle( hJob );
   QProcess::kill();
#endif
}

void ChildProcess::terminate()
{
#if defined Q_OS_UNIX
   ::killpg(::getpgid(pid()), SIGTERM);
#else
   CloseHandle( hJob );
   QProcess::terminate();
#endif
}
