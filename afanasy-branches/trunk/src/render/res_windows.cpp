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

struct io
{
  LARGE_INTEGER BytesRead;
  LARGE_INTEGER BytesWritten;
  LARGE_INTEGER ReadTime;
  LARGE_INTEGER WriteTime;
  LARGE_INTEGER IdleTime;
  DWORD         ReadCount;
  DWORD         WriteCount;
  DWORD         QueueDepth;
  DWORD         SplitCount;
  LARGE_INTEGER QueryTime;
} io0, io1;

int now = 0;

/*
void WINAPI GetSystemTimeAsFileTime(
  __out  LPFILETIME lpSystemTimeAsFileTime
);
typedef struct _FILETIME {
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
} FILETIME, *PFILETIME;
*/
ULONGLONG g_time_prev = 0;

void GetResources( af::Host & host, af::HostRes & hres, bool verbose)
{
#ifdef AFOUTPUT
printf("\nGetResources:\n");
#endif //AFOUTPUT

    // Time interval between calls calculation:
    FILETIME ftime_now;
    GetSystemTimeAsFileTime( &ftime_now);
    ULARGE_INTEGER time_now;
    time_now.HighPart = ftime_now.dwHighDateTime;
    time_now.LowPart  = ftime_now.dwLowDateTime;
    ULONGLONG time_delta = time_now.QuadPart - g_time_prev;
    if( time_delta == 0 )
        time_delta = 1;
    double time_interval_sec = double( time_delta ) / 10000000.0;
    g_time_prev = time_now.QuadPart;

    /* Will be set to 1 after first run. */
    static unsigned s_init = 0;

   //
   // CPU info:
   //
   if( !s_init)
   {
      // number of pocessors
      SYSTEM_INFO sysinfo;
      GetSystemInfo( &sysinfo);
      hres.cpu_num = sysinfo.dwNumberOfProcessors;

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
         hres.cpu_mhz = dwMHz;
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
         hres.cpu_mhz = 1000;
      }
   }

   //
   // Memory & Swap total and usage:
   //
   {
   MEMORYSTATUSEX statex;
   statex.dwLength = sizeof( statex);
   GlobalMemoryStatusEx( &statex);
   if( !s_init )
   {
      hres.mem_total_mb  = int( statex.ullTotalPhys     >> 20 );
      hres.swap_total_mb = int( statex.ullTotalPageFile >> 20 );
   }
   hres.mem_free_mb    = int( statex.ullAvailPhys >> 20 );
   hres.mem_cached_mb  = 0;
   hres.mem_buffers_mb = 0;
   hres.swap_used_mb   = int(( statex.ullTotalPageFile - statex.ullAvailPageFile ) >> 20);
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
#ifdef AFOUTPUT
   printf("idleTime   = %llu\n", (unsigned long long)(cpu_now->idle.QuadPart));
   printf("kernelTime = %llu\n", (unsigned long long)(cpu_now->kernel.QuadPart));
   printf("userTime   = %llu\n", (unsigned long long)(cpu_now->user.QuadPart));
#endif //AFOUTPUT
   // Check counters overflow:
   if(( cpu_now->idle.QuadPart   >= cpu_last->idle.QuadPart    ) &&
      ( cpu_now->kernel.QuadPart >= cpu_last->kernel.QuadPart  ) &&
      ( cpu_now->user.QuadPart   >= cpu_last->user.QuadPart    ))
   {
      unsigned long long idle   = cpu_now->idle.QuadPart   - cpu_last->idle.QuadPart;
      unsigned long long kernel = cpu_now->kernel.QuadPart - cpu_last->kernel.QuadPart;
      unsigned long long user   = cpu_now->user.QuadPart   - cpu_last->user.QuadPart;
      unsigned long long system = kernel - idle;
      unsigned long long total  = kernel + user;
#ifdef AFOUTPUT
      printf("idle   = %9llu\n", idle);
      printf("kernel = %9llu\n", kernel);
      printf("user   = %9llu\n", user);
      printf("total  = %9llu\n", total);
#endif //AFOUTPUT
      if( total != 0 )
      {
         hres.cpu_user    = uint8_t(( user   * 100) / total);
         hres.cpu_system  = uint8_t(( system * 100) / total);
         hres.cpu_idle    = uint8_t(( idle   * 100) / total);
         hres.cpu_nice    = 0;
         hres.cpu_iowait  = 0;
         hres.cpu_irq     = 0;
         hres.cpu_softirq = 0;
      }
   }

   double loadavg[3] = { 0, 0, 0 };
//   int nelem = getloadavg( loadavg, 3);
//   if( nelem < 2 ) loadavg[1] = loadavg[0];
//   if( nelem < 3 ) loadavg[2] = loadavg[1];
   if( loadavg[0] > 25.0 ) loadavg[0] = 25.0;
   if( loadavg[1] > 25.0 ) loadavg[1] = 25.0;
   if( loadavg[2] > 25.0 ) loadavg[2] = 25.0;
   hres.cpu_loadavg[0] = unsigned( 10.0 * loadavg[0]);
   hres.cpu_loadavg[1] = unsigned( 10.0 * loadavg[1]);
   hres.cpu_loadavg[2] = unsigned( 10.0 * loadavg[2]);
   }//cpu

   //
   // HDD
   //
   {// space:
   static char * directory = NULL;
   if((!s_init) && (af::Environment::getRenderHDDSpacePath() != "/"))
   {
      static char path[4096];
      sprintf_s( path, "%s", af::Environment::getRenderHDDSpacePath().c_str());
      directory = path;
      printf("HDD Space Path = '%s'\n", directory);
   }
   ULARGE_INTEGER totalNumberOfBytes, totalNumberOfFreeBytes;
   if( GetDiskFreeSpaceEx( directory, NULL, &totalNumberOfBytes, &totalNumberOfFreeBytes))
   {
      hres.hdd_total_gb = int( totalNumberOfBytes.QuadPart >> 30 );
      hres.hdd_free_gb  = int( totalNumberOfFreeBytes.QuadPart >> 30 );
   }
   else
   {
      hres.hdd_total_gb = 0;
      hres.hdd_free_gb  = 0;
   }
   }
   {// io:
   hres.hdd_rd_kbsec = 0;
   hres.hdd_wr_kbsec = 0;
   hres.hdd_busy = -1;

   HANDLE hDevice;               // handle to the drive to be examined 
   BOOL bResult;                 // results flag
   DWORD junk;                   // discard results

   hDevice = CreateFile(TEXT("\\\\.\\PhysicalDrive0"),  // drive to open
                    0,                // no access to the drive
                    FILE_SHARE_READ | // share mode
                    FILE_SHARE_WRITE, 
                    NULL,             // default security attributes
                    OPEN_EXISTING,    // disposition
                    0,                // file attributes
                    NULL);            // do not copy file attributes

   if( hDevice != INVALID_HANDLE_VALUE) // cannot open the drive
   {
      DISK_PERFORMANCE dp;
      bResult = DeviceIoControl(hDevice,  // device to be queried
             IOCTL_DISK_PERFORMANCE,  // operation to perform
                             NULL, 0, // no input buffer
                            &dp, sizeof(dp),     // output buffer
                            &junk,                 // # bytes returned
                            (LPOVERLAPPED) NULL);  // synchronous I/O
      if( bResult )
      {
                io * io_last = &io0; io * io_now = &io1;
         if( now ) { io_last = &io1;      io_now = &io0; }

         if((!s_init) && verbose )
         {
            printf("Storage Manager Name = ");
            for ( int j = 0; j < 8; j++) printf("%c", dp.StorageManagerName[j]);
            printf("\n");
         }

         io_now->BytesRead    = dp.BytesRead;
         io_now->BytesWritten = dp.BytesWritten;
         io_now->ReadTime     = dp.ReadTime;
         io_now->WriteTime    = dp.WriteTime;
         io_now->IdleTime     = dp.IdleTime;
         io_now->ReadCount    = dp.ReadCount;
         io_now->WriteCount   = dp.WriteCount;
         io_now->QueueDepth   = dp.QueueDepth;
         io_now->SplitCount   = dp.SplitCount;
         io_now->QueryTime    = dp.QueryTime;
#ifdef AFOUTPUT
         printf("Total BytesRead    = %llu\n", (unsigned long long)(io_now->BytesRead.QuadPart));
         printf("Total BytesWritten = %llu\n", (unsigned long long)(io_now->BytesWritten.QuadPart));
#endif //AFOUTPUT
         // Check for bytes counters overflow:
         if(( io_now->BytesRead.QuadPart     >= io_last->BytesRead.QuadPart   ) &&
            ( io_now->BytesWritten.QuadPart  >= io_last->BytesWritten.QuadPart))
         {
            unsigned long long BytesRead    = io_now->BytesRead.QuadPart    - io_last->BytesRead.QuadPart;
            unsigned long long BytesWritten = io_now->BytesWritten.QuadPart - io_last->BytesWritten.QuadPart;
#ifdef AFOUTPUT
            printf("Delta BytesRead    = %llu\n", BytesRead);
            printf("Delta BytesWritten = %llu\n", BytesWritten);
#endif //AFOUTPUT
            hres.hdd_rd_kbsec = int( double( BytesRead   ) / 1024.0 / time_interval_sec);
            hres.hdd_wr_kbsec = int( double( BytesWritten) / 1024.0 / time_interval_sec);
         }

         // Check for time counters overflow:
         if(( io_now->ReadTime.QuadPart  >= io_last->ReadTime.QuadPart  ) &&
            ( io_now->WriteTime.QuadPart >= io_last->WriteTime.QuadPart ) &&
            ( io_now->IdleTime.QuadPart  >= io_last->IdleTime.QuadPart  ))
         {
            unsigned long long ReadTime  = io_now->ReadTime.QuadPart  - io_last->ReadTime.QuadPart;
            unsigned long long WriteTime = io_now->WriteTime.QuadPart - io_last->WriteTime.QuadPart;
            unsigned long long IdleTime  = io_now->IdleTime.QuadPart  - io_last->IdleTime.QuadPart;
            unsigned long long TotalTime = ReadTime + WriteTime + IdleTime;
            if( TotalTime != 0 )
            {
#ifdef AFOUTPUT
               printf("ReadTime  = %9llu\nWriteTime = %9llu\nIdleTime  = %9llu\nTotalTime = %9llu\n", ReadTime, WriteTime, IdleTime, TotalTime);
#endif //AFOUTPUT
               int busy = int((( ReadTime + WriteTime ) * 100 ) / TotalTime );
               if( busy < 0   ) busy = 0;
               if( busy > 100 ) busy = 100;

               hres.hdd_busy = busy;
            }
         }

      }

      CloseHandle(hDevice);
   }
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
         if((!s_init) && verbose)
         {
            printf("IF#%2d InterfaceName:\t %ws\n", row->dwType, row->wszName);
            printf("IF#%2d Description:\t ", row->dwType);
            for ( int j = 0; j < (int) row->dwDescrLen; j++) printf("%c", row->bDescr[j]);
            printf("\n");
         }
         if( verbose ) printf("IF#%2d: Octets in/out[%d] = %9u %9u\n",  row->dwType, now, row->dwInOctets, row->dwOutOctets);
         net_now->recv += row->dwInOctets  >> 10;
         net_now->send += row->dwOutOctets >> 10;
      }
      int recv = net_now->recv - net_last->recv;
      int send = net_now->send - net_last->send;
      if( recv >= 0 ) hres.net_recv_kbsec = double(recv) / time_interval_sec;
      if( send >= 0 ) hres.net_send_kbsec = double(send) / time_interval_sec;
   }
   if( ifTable) FREE( ifTable);
   }//network

   now = 1 - now;

	s_init = 1;

//hres.stdOut(false);
}
#endif
