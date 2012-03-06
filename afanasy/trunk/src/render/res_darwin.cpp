#ifdef MACOSX
#include "res.h"

#include <fcntl.h>
#include <mach/host_info.h>
#include <mach/mach.h>
#include <mach/vm_statistics.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/sysctl.h>

/* All this IOKit stuff is for the disk drives statistics. */
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOBlockStorageDriver.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/IOBSD.h>

#include <algorithm>

/* WARNING: from gataddrs man pages on Mac Os X
   If both <net/if.h> and <ifaddrs.h> are being included, <net/if.h> must be
   included before <ifaddrs.h>.
*/
#include <net/if.h>
#include <ifaddrs.h>

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

host_cpu_load_info_data_t s_cpu_load_stats = {0};

struct network_statistics_s
{
   uint64_t total_recv;
   uint64_t total_send;
} network_statistics = { 0, 0 };

/* most drives we will record */
#define MAXDRIVES 16

/* largest drive name we allow */
#define MAXDRIVENAME 31

static mach_port_t s_master_port;

static unsigned s_init = 0;
static unsigned s_num_drives = 0;
static struct timeval s_cur_time, s_last_time;

/* Statistics from one disk drive. */
struct drivestats
{
   uint64_t read, write;
   /* uint64_t total_time; */
} s_disk_stats = {0};

/* declarations, this is taken from iostat.h (DARWIN) . */
static bool get_drive_stats( uint64_t &o_read, uint64_t &o_write);
static double compute_etime( struct timeval cur_time, struct timeval prev_time);

void GetResources( af::Host & host, af::HostRes & hres, bool verbose)
{
   /* Will be set to 1 after first run. */
   static unsigned s_init = 0;

   /* Some variables that we should get only once. NOTE: not sure this
      really worth it. */
   static uint64_t s_hz = 0;
   static uint64_t s_physical_memory = 0;
   static unsigned s_pagesize = 0;

   /*
      Start by getting the time interval since the last 'GetResources' call. 
      if this is the first call, the time interval is computed since the 
      last boot of this machine. This means the GetResources will return the
      average stats since the boot of this machine when called for the first
      time.
   */

   s_last_time = s_cur_time;
   gettimeofday(&s_cur_time, NULL);
   double etime = compute_etime(s_cur_time, s_last_time);

   if( !s_init ) /* not thread safe but not really important. */
   {
      /*
       * This is the first time we are called.
       * Set the busy time to the system boot time, so the stats are
       * calculated since system boot.
       */
      size_t sizeof_cur_time = sizeof(s_cur_time);
      if (sysctlbyname("kern.boottime", &s_cur_time, &sizeof_cur_time, NULL, 0) == -1) 
      {
         perror( "sysctlbyname(\"kern.bootime\",...) failed: " );
         return;
      }

      size_t size = sizeof(s_hz);
      if( sysctlbyname("hw.cpufrequency", &s_hz, &size, NULL, 0) != 0)
      {
         perror( "sysctlbyname(\"hw.cpufrequency\",..) failed: " );
         s_hz = 1000 << 20;
      }

      size = sizeof(s_physical_memory);
      if( sysctlbyname("hw.memsize", &s_physical_memory, &size, NULL, 0) != 0 )
      {
         perror( "sysctlbyname(\"hw.memsize\",..) failed: " );
      }
      else
      {
         s_physical_memory /= (1024*1024);
      }

      s_pagesize = getpagesize();

      s_init = 1;
   }

   //
   // CPU info:
   //

   /* This should be a constant obviously, unless you plan to remove a CPU
      while rendering. :) */
   static unsigned num_processors = sysconf(_SC_NPROCESSORS_ONLN);
   hres.cpu_num = num_processors;
   hres.cpu_mhz = s_hz >> 20;

   //
   // Memory & Swap total and usage:
   //
   {
      vm_statistics_data_t vm_stats;
      unsigned memtotal=0, memfree=0;
      unsigned membuffers=0, memcached=0;

      memtotal = s_physical_memory;
      unsigned count = HOST_VM_INFO_COUNT;

      if( host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&vm_stats, &count) == KERN_SUCCESS)
      {
         memfree = vm_stats.free_count;
         membuffers = 0;
         memcached = vm_stats.inactive_count;
      }
      else
      {
        perror("host_satistics(HOST_VM_INFO) failed");
      }

      /* size of data in bytes. */
      xsw_usage vmusage = {0};
      size_t size = sizeof(vmusage);
      if( sysctlbyname("vm.swapusage", &vmusage, &size, NULL, 0)!=0 )
      {
         perror( "unable to get swap usage by calling sysctlbyname(\"vm.swapusage\",...)" ); 
      }

      /* A convertion factor to bring us to MBs */
      hres.mem_total_mb = s_physical_memory;
      hres.swap_total_mb = vmusage.xsu_total >> 20;
      hres.mem_free_mb = (memfree * s_pagesize) >> 20;
      hres.mem_cached_mb = (memcached  * s_pagesize) >> 20;
      hres.mem_buffers_mb = (membuffers * s_pagesize) >> 20;
      hres.swap_used_mb = vmusage.xsu_used  >> 20;
   }

   //
   // CPU usage:
   //
   {
      unsigned int count = HOST_CPU_LOAD_INFO_COUNT;
      host_cpu_load_info_data_t current_cpu_load;

      if( host_statistics(
            mach_host_self(), HOST_CPU_LOAD_INFO,
            (host_info_t)&current_cpu_load, &count) == KERN_SUCCESS)
      {
         /* NOTE: we seem to have no 'iowait', "sofirq" and 'irq' on Mac Os X ? */
         unsigned states[] = { CPU_STATE_USER, CPU_STATE_IDLE, CPU_STATE_SYSTEM,
            CPU_STATE_NICE };

         uint64_t intervals[ sizeof(states)/sizeof(states[0]) ];
         uint64_t total = 0;

         for( unsigned i=0; i<sizeof(states)/sizeof(states[0]); i++ )
         {
            unsigned s = states[i];
            intervals[i] =
               current_cpu_load.cpu_ticks[s] - s_cpu_load_stats.cpu_ticks[s];

            total += intervals[i];

            /* Update our counts for next iteration. */
            s_cpu_load_stats.cpu_ticks[i] = current_cpu_load.cpu_ticks[i];
         }

         if( total == 0 )
            total = 1;

         hres.cpu_user    = ( 100 * intervals[0] ) / total;
         hres.cpu_idle    = ( 100 * intervals[1] ) / total;
         hres.cpu_system  = ( 100 * intervals[2] ) / total;
         hres.cpu_nice    = ( 100 * intervals[3] ) / total;
         hres.cpu_iowait  = 0;
         hres.cpu_irq     = 0;
         hres.cpu_softirq = 0;
      }
      else
      {
         AFERRPE("GetResources: host_satistics( HOST_CPU_LOAD_INFO) failure:");
      }

      // CPU Load Average:
      double loadavg[3] = { 0, 0, 0 };
      int nelem = getloadavg( loadavg, 3);

      /* FIXME: we need to put this in 0-255 range because we transmit these
         values as 8 bit integers which is not really good. */
      for( unsigned i=0; i<nelem; i++ )
      {
         double la = 10.0 * loadavg[0];

         if( la > 255 )
            la = 255;
         else if( la < 0 )
            la = 0;

         hres.cpu_loadavg[i] = la;
      }
   }

   //
   // HDD space:
   //
   {
      static char path[MAXPATHLEN+1];
      snprintf( path, MAXPATHLEN, "%s",
         af::Environment::getRenderHDDSpacePath().c_str());

      struct statfs fsd;
      if( statfs(path, &fsd) >= 0 )
      {
         hres.hdd_total_gb = ((fsd.f_blocks >> 10) * fsd.f_bsize) >> 20;
         hres.hdd_free_gb  = ((fsd.f_bfree  >> 10) * fsd.f_bsize) >> 20;
      }
      else
      {
         perror( "statfs() failed : ");
      }
   }

   /*
      Network.

      We sum the statistics of all AF_LINK interfaces.
   */

   hres.net_recv_kbsec = 0;
   hres.net_send_kbsec = 0;

   struct ifaddrs *addr;
   if( getifaddrs( &addr ) == 0 )
   {
      uint64_t net_recv = 0,  net_send = 0;

      for( struct ifaddrs *it = addr; it; it = it->ifa_next )
      {
         /* We're looking for a link level address. */
         if( !it->ifa_addr || it->ifa_addr->sa_family != AF_LINK )
            continue;

         /* This should not happen but there are some pretty wild stuff
            out there so better be careful. */
         if( !it->ifa_name || !it->ifa_data )
            continue;

         /* ifa_data holds statistics for AF_LINK interfaces. */
         struct if_data *stats = (struct if_data *) it->ifa_data;

         net_recv += stats->ifi_ibytes;
         net_send += stats->ifi_obytes;
      }

      int64_t recv_interval = net_recv - network_statistics.total_recv;
      int64_t send_interval = net_send - network_statistics.total_send;

      network_statistics.total_recv = net_recv;
      network_statistics.total_send = net_send;

      hres.net_recv_kbsec = recv_interval / (etime*1024);
      hres.net_send_kbsec = send_interval / (etime*1024);

      freeifaddrs( addr );
   }

   uint64_t read=0, write=0;
   if( get_drive_stats(read, write) ) 
   {
      uint64_t read_interval = read - s_disk_stats.read;
      uint64_t write_interval = write - s_disk_stats.write;

      s_disk_stats.read = read;
      s_disk_stats.write = write;

      hres.hdd_rd_kbsec = read_interval / (etime*1024);
      hres.hdd_wr_kbsec = write_interval / (etime*1024);
   }
   else
   {
      hres.hdd_rd_kbsec = hres.hdd_wr_kbsec = 0;
   }
}

static bool get_drive_stats( uint64_t &o_read, uint64_t &o_write)
{
	mach_port_t master_port;

	/*  Get the I/O Kit communication handle.  */
	IOMasterPort(bootstrap_port, &master_port);

	/* Get an iterator for IOMedia objects.  */
	CFMutableDictionaryRef match = IOServiceMatching("IOMedia");
	CFDictionaryAddValue( match, CFSTR(kIOMediaWholeKey), kCFBooleanTrue );

	io_iterator_t drivelist;
	kern_return_t status =
		IOServiceGetMatchingServices(master_port, match, &drivelist);

	if (status != KERN_SUCCESS)
		return false;

	CFNumberRef number;
	CFDictionaryRef properties;
	CFDictionaryRef statistics;

	uint64_t value;
	uint64_t total_time = 0;

	io_registry_entry_t drive, parent;

	while( drive = IOIteratorNext(drivelist) )
   {
      /* get drive's parent */
      kern_return_t status =
         IORegistryEntryGetParentEntry(drive, kIOServicePlane, &parent);

      /* We don't need this one anymore. */
      IOObjectRelease( drive );

      if( status != KERN_SUCCESS )
         continue;

      if( !IOObjectConformsTo(parent, "IOBlockStorageDriver") )
      {
         /* Not a block storage device, skip. NOTE: do we need to
            go to "more parents" if this one is not a block device ?? */
         IOObjectRelease( parent );
         continue;
      }

      /* get drive properties */
      status =
         IORegistryEntryCreateCFProperties(
               parent,
               (CFMutableDictionaryRef *)&properties,
               kCFAllocatorDefault,
               kNilOptions);

      /* We don't need this one anymore. */
      IOObjectRelease( parent );

      if( status != KERN_SUCCESS )
      {
         continue;
      }

      /* get statistics from properties */
      statistics = (CFDictionaryRef)
         CFDictionaryGetValue(
               properties, CFSTR(kIOBlockStorageDriverStatisticsKey));

      if( !statistics )
      {
         CFRelease(properties);
         continue;
      }

      if ((number = (CFNumberRef)CFDictionaryGetValue(statistics,
                  CFSTR(kIOBlockStorageDriverStatisticsBytesReadKey))))
      {
         CFNumberGetValue(number, kCFNumberSInt64Type, &value);
         o_read += value;
      }

      if ((number = (CFNumberRef)CFDictionaryGetValue(statistics,
                  CFSTR(kIOBlockStorageDriverStatisticsBytesWrittenKey))))
      {
         CFNumberGetValue(number, kCFNumberSInt64Type, &value);
         o_write += value;
      }
   }

   return true;
}

static double
compute_etime(struct timeval cur_time, struct timeval prev_time)
{
	struct timeval busy_time;
	uint64_t busy_usec;
	long double etime;

	timersub(&cur_time, &prev_time, &busy_time);

	busy_usec = busy_time.tv_sec;  

	busy_usec *= 1000000;          
	busy_usec += busy_time.tv_usec;
	etime = busy_usec;
	etime /= 1000000;

	return(etime);
}



#endif
