#ifdef WINNT
#include "res.h"

#include <Winbase.h>

//#include "../libafanasy/environment.h"

struct cpu
{
   ULARGE_INTEGER user;
   ULARGE_INTEGER system;
   ULARGE_INTEGER idle;
}cpu0,cpu1;
int now = 0;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

void GetResources( af::Host & host, af::HostRes & hres, bool getConstants)
{
   //
   // CPU info:
   //
   if( getConstants)
   {
      // number of pocessors
      SYSTEM_INFO sysinfo;
      GetSystemInfo( &sysinfo);
      host.cpu_num = sysinfo.dwNumberOfProcessors;

      // frequency of first pocessor
      HKEY hKey;
      char Buffer[_MAX_PATH];
      DWORD BufSize = _MAX_PATH;
      DWORD dwMHz = _MAX_PATH;
      long lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                        0,
                        KEY_READ,
                        &hKey);
      if( lError == ERROR_SUCCESS)
      {
         RegQueryValueEx( hKey, "~MHz", NULL, NULL, (LPBYTE) &dwMHz, &BufSize);
         host.cpu_mhz = dwMHz;
      }
      else
	   {
         FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                         NULL,
                         lError,
                         0,
                         Buffer,
                         _MAX_PATH,
                         0);
         AFERRAR("%s", Buffer);
         host.cpu_mhz = 1000;
      }
   }

   //
   // Memory & Swap total and usage:
   //
   {
   MEMORYSTATUSEX statex;
   statex.dwLength = sizeof( statex);
   GlobalMemoryStatusEx( &statex);
   if( getConstants )
   {
      host.mem_mb  = statex.ullTotalPhys >> 20;
      host.swap_mb = statex.ullTotalPageFile >> 20;
   }
   hres.mem_free_mb    = statex.ullAvailPhys >> 20;
   hres.mem_cached_mb  = 0;
   hres.mem_buffers_mb = 0;
   hres.swap_used_mb   = ( statex.ullTotalPageFile - statex.ullAvailPageFile ) >> 20;
   }//memory

   //
   // CPU usage:
   //
   {
                cpu * cpu_last = &cpu0; cpu * cpu_now = &cpu1;
   if( now ) {        cpu_last = &cpu1;       cpu_now = &cpu0; }
   FILETIME idleTime, kernelTime, userTime;
   GetSystemTimes( &idleTime, &kernelTime, &userTime);
   cpu_now->idle.HighPart    = idleTime.dwHighDateTime;
   cpu_now->idle.LowPart     = idleTime.dwLowDateTime;
   cpu_now->system.HighPart  = kernelTime.dwHighDateTime;
   cpu_now->system.LowPart   = kernelTime.dwLowDateTime;
   cpu_now->user.HighPart    = userTime.dwHighDateTime;
   cpu_now->user.LowPart     = userTime.dwLowDateTime;
/*
   printf("idleTime   = %u %u\n", idleTime.dwHighDateTime, idleTime.dwLowDateTime);
   printf("kernelTime = %u %u\n", kernelTime.dwHighDateTime, kernelTime.dwLowDateTime);
   printf("userTime   = %u %u\n", userTime.dwHighDateTime, userTime.dwLowDateTime);
   printf("idleTime   = %u\n", cpu_now->idle.QuadPart);
   printf("kernelTime = %u\n", cpu_now->system.QuadPart);
   printf("userTime   = %u\n", cpu_now->user.QuadPart);
*/
   int idle   = cpu_now->idle.QuadPart   - cpu_last->idle.QuadPart;
   int system = cpu_now->system.QuadPart - cpu_last->system.QuadPart;
   int user   = cpu_now->user.QuadPart   - cpu_last->user.QuadPart;
   int total  = idle + system + user;
/*
   printf("delta idle   = %u\n", idle);
   printf("delta kernel = %u\n", system);
   printf("delta user   = %u\n", user);
   printf("total  = %u\n", total);
*/
   hres.cpu_user    = ( 100 * user    ) / total;
   hres.cpu_system  = ( 100 * system  ) / total;
   hres.cpu_idle    = ( 100 * idle    ) / total;
   hres.cpu_nice    = 0;
   hres.cpu_iowait  = 0;
   hres.cpu_irq     = 0;
   hres.cpu_softirq = 0;

   double loadavg[3] = { 0, 0, 0 };
//   int nelem = getloadavg( loadavg, 3);
//   if( nelem < 2 ) loadavg[1] = loadavg[0];
//   if( nelem < 3 ) loadavg[2] = loadavg[1];
   if( loadavg[0] > 25.0 ) loadavg[0] = 25.0;
   if( loadavg[1] > 25.0 ) loadavg[1] = 25.0;
   if( loadavg[2] > 25.0 ) loadavg[2] = 25.0;
   hres.cpu_loadavg1 = unsigned( 10.0 * loadavg[0]);
   hres.cpu_loadavg2 = unsigned( 10.0 * loadavg[1]);
   hres.cpu_loadavg3 = unsigned( 10.0 * loadavg[2]);
   }//cpu

   //
   // HDD space:
   //
   {
   ULARGE_INTEGER totalNumberOfBytes, totalNumberOfFreeBytes;
   GetDiskFreeSpaceEx( NULL, NULL, &totalNumberOfBytes, &totalNumberOfFreeBytes);
   if( getConstants) host.hdd_gb = totalNumberOfBytes.QuadPart >> 30;
   hres.hdd_free_gb  = totalNumberOfFreeBytes.QuadPart >> 30;
   }//hdd

   //
   // Network:
   //
   {
   hres.net_recv_kbsec = 0;
   hres.net_send_kbsec = 0;
   }//network

   now = 1 - now;
}
#endif
