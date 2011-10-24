#include "childprocess.h"

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

#ifdef UNIX
#include <signal.h>
//#include <sys/prctl.h>
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
#ifdef UNIX
//printf("void ChildProcess::setupChildProcess():\n");
   if( setsid() == -1) AFERRPE("setsid")
   int nicenew = nice( af::Environment::getRenderNice());
   if( nicenew == -1) AFERRPE("nice")
//   prctl( PR_SET_PDEATHSIG, SIGTERM);
#else
   process_info = pid();
   SetPriorityClass( process_info->hProcess, BELOW_NORMAL_PRIORITY_CLASS);
   if( AssignProcessToJobObject( hJob, process_info->hProcess) == false)
      AFERROR("AssignProcessToJobObject failed.\n");
   ResumeThread( process_info->hThread); // | CREATE_SUSPENDED
#endif
}

void ChildProcess::terminate()
{
#ifdef UNIX
   killpg( getpgid(pid()), SIGTERM);
#else
   CloseHandle( hJob );
//   QProcess::terminate();
#endif
}

void ChildProcess::kill()
{
#ifdef UNIX
   killpg( getpgid(pid()), SIGKILL);
#else
   CloseHandle( hJob );
//   QProcess::kill();
#endif
}
