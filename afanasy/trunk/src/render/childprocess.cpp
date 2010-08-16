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
#ifdef WINNT
   hJob = CreateJobObject( NULL, NULL);
   JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
   jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE; //0x00002000;
   if( SetInformationJobObject( hJob, JobObjectExtendedLimitInformation, &jeli, sizeof( jeli ) ) == 0)
      AFERROR("SetInformationJobObject failed.\n");
#endif
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
   process_info = pid();
   SetPriorityClass( process_info->hProcess, BELOW_NORMAL_PRIORITY_CLASS);
   if( AssignProcessToJobObject( hJob, process_info->hProcess) == false)
      AFERROR("AssignProcessToJobObject failed.\n");
   ResumeThread( process_info->hThread); // | CREATE_SUSPENDED
#endif
}

void ChildProcess::kill()
{
#if defined Q_OS_UNIX
   ::killpg(::getpgid(pid()), SIGKILL);
#else
   CloseHandle( hJob );
//   QProcess::kill();
#endif
}

void ChildProcess::terminate()
{
#if defined Q_OS_UNIX
   ::killpg(::getpgid(pid()), SIGTERM);
#else
   CloseHandle( hJob );
//   QProcess::terminate();
#endif
}
