#ifdef WINNT
#include "res.h"

#include <Winbase.h>
#include <Iphlpapi.h>

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

struct cpu
{
   ULARGE_INTEGER idle;
   ULARGE_INTEGER kernel;
   ULARGE_INTEGER user;
} cpu0, cpu1;

struct net
{
   DWORD send;
   DWORD recv;
} net0, net1;

int now = 0;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

void GetResources( af::Host & host, af::HostRes & hres, bool getConstants, bool verbose)
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
   if( now ) { cpu_last = &cpu1;       cpu_now = &cpu0; }
   FILETIME idleTime, kernelTime, userTime;
   GetSystemTimes( &idleTime, &kernelTime, &userTime);
   cpu_now->idle.HighPart    = idleTime.dwHighDateTime;
   cpu_now->idle.LowPart     = idleTime.dwLowDateTime;
   cpu_now->kernel.HighPart  = kernelTime.dwHighDateTime;
   cpu_now->kernel.LowPart   = kernelTime.dwLowDateTime;
   cpu_now->user.HighPart    = userTime.dwHighDateTime;
   cpu_now->user.LowPart     = userTime.dwLowDateTime;
/*
   printf("idleTime   = %u %u\n", idleTime.dwHighDateTime, idleTime.dwLowDateTime);
   printf("kernelTime = %u %u\n", kernelTime.dwHighDateTime, kernelTime.dwLowDateTime);
   printf("userTime   = %u %u\n", userTime.dwHighDateTime, userTime.dwLowDateTime);
   printf("idleTime   = %u\n", cpu_now->idle.QuadPart);
   printf("kernelTime = %u\n", cpu_now->kernel.QuadPart);
   printf("userTime   = %u\n", cpu_now->user.QuadPart);
*/
   int idle   = cpu_now->idle.QuadPart   - cpu_last->idle.QuadPart;
   int kernel = cpu_now->kernel.QuadPart - cpu_last->kernel.QuadPart;
   int user   = cpu_now->user.QuadPart   - cpu_last->user.QuadPart;
   int system   =  kernel - idle;
   int total100 = (kernel + user) / 100;
   if( total100 < 1 ) total100 = 1;
/*
   printf("idle   = %9d\n", idle);
   printf("kernel = %9d\n", kernel);
   printf("user   = %9d\n", user);
   printf("total  = %9d\n", total);
*/
   hres.cpu_user    = user   / ( total100 );
   hres.cpu_system  = system / ( total100 );
   hres.cpu_idle    = idle   / ( total100 );
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
   // Declare and initialize variables
   MIB_IFTABLE * ifTable;
   static DWORD dwSize = sizeof( MIB_IFTABLE);
   DWORD dwRetVal = 0;

   // Allocate memory for our pointers
   ifTable = (MIB_IFTABLE*)MALLOC(dwSize);

   // Make an initial call to GetIfTable to get the
   // necessary size into the dwSize variable
   if(( dwRetVal = GetIfTable(ifTable, &dwSize, FALSE)) == ERROR_INSUFFICIENT_BUFFER )
   {
      FREE( ifTable);
      if( verbose ) printf("Size of MIB_IFTABLE = %d\n",dwSize);
      dwSize *= 2;
      ifTable = (MIB_IFTABLE *)MALLOC( dwSize);
      dwRetVal = GetIfTable(ifTable, &dwSize, FALSE);
   }

   // Make a second call to GetIfTable to get the 
   // actual data we want
   if( dwRetVal == NO_ERROR )
   {
            net * net_last = &net0; net * net_now = &net1;
      if( now ) { net_last = &net1;       net_now = &net0; }
      net_now->recv = 0;
      net_now->send = 0;
      std::list<int>iftypes;
      // Iterare trough devices:
      for( int i = 0; i < (int)ifTable->dwNumEntries; i++)
      {
         MIB_IFROW * row = & ifTable->table[i];
         // Skip loopback interface:
         if( row->dwType == IF_TYPE_SOFTWARE_LOOPBACK) continue;
         // Skip 'empty' interfaces:
         if((row->dwInOctets == 0) && (row->dwInOctets == 0)) continue;
         // Skip calculated interfaces:
         bool calculated = false;
         for( std::list<int>::const_iterator it = iftypes.begin(); it != iftypes.end(); it++)
         {
            if( *it == row->dwType)
            {
               calculated = true;
               break;
            }
         }
         if( calculated ) continue;
         iftypes.push_back( row->dwType);
         if( getConstants && verbose)
         {
            printf("IF#%2d InterfaceName:\t %ws\n", row->dwType, row->wszName);
            printf("IF#%2d Description:\t ", row->dwType);
            for ( int j = 0; j < (int) row->dwDescrLen; j++) printf("%c", row->bDescr[j]);
            printf("\n");
         }
         if( verbose ) printf("IF#%2d: Octets in/out[%d] = %9d %9d\n",  row->dwType, now, row->dwInOctets, row->dwOutOctets);
         net_now->recv += row->dwInOctets  >> 10;
         net_now->send += row->dwOutOctets >> 10;
      }
      int recv = net_now->recv - net_last->recv;
      int send = net_now->send - net_last->send;
      if( recv >= 0 ) hres.net_recv_kbsec = recv / af::Environment::getRenderUpdateSec();
      if( send >= 0 ) hres.net_send_kbsec = send / af::Environment::getRenderUpdateSec();
   }
   if( ifTable) FREE( ifTable);
   }//network

   now = 1 - now;

//hres.stdOut(false);
}
#endif
