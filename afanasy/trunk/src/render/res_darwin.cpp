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

host_cpu_load_info_data_t cpuload;

#define pagetok(size) ((size)<<pageshift)
vm_statistics_data_t vm_stats;
unsigned int maxmem;
int pageshift  =  0;
int pagesize   =  0;
int swappgsin  = -1;
int swappgsout = -1;

struct res
{
   int user;
   int nice;
   int system;
   int idle;
   int iowait;
   int irq;
   int softirq;
}r0,r1;

struct net
{
   int recv;
   int send;
} network_statistics[2] = { {0,0}, {0,0} };

unsigned int memtotal, memfree, membuffers, memcached, swaptotal, swapfree;

int now = 0;

void GetResources( af::Host & host, af::HostRes & hres, bool getConstants, bool verbose)
{
   //
   // CPU info:
   //
   if( getConstants)
   {
      // number of pocessors
      host.cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
      // frequency of first pocessor
      unsigned hz;
      size_t size = sizeof( hz);
      if( sysctlbyname("hw.cpufrequency", &hz, &size, NULL, 0) == -1) perror("sysctlbyname");
      else host.cpu_mhz = hz >> 20;
   }
   //
   // Memory & Swap total and usage:
   //
   {
      if( getConstants )
      {
         size_t size = sizeof(maxmem);
         sysctlbyname("hw.physmem", &maxmem, &size, NULL, 0);
         pagesize = getpagesize();

         memtotal = maxmem >> 10;
         printf("Memory Total = %u KBytes\n", memtotal);
         printf("Memory Page Size = %u Bytes\n", pagesize);

         pageshift = 0;
         while(( pagesize >>= 1) > 0) pageshift++;
         pageshift -= 10;
         printf("Memory Page Shift = %d\n", pageshift);
      }
      unsigned int count = HOST_VM_INFO_COUNT;
      if( host_statistics( mach_host_self(), HOST_VM_INFO, (host_info_t)&vm_stats, &count) == KERN_SUCCESS)
      {
         memfree    = pagetok( vm_stats.free_count);
         membuffers = 0;
         memcached  = pagetok( vm_stats.inactive_count);
         if( swappgsin < 0)
         {
            swapfree = 0;
         }
         else
         {
            int swapin  = pagetok( vm_stats.pageins  - swappgsin );
            int swapout = pagetok( vm_stats.pageouts - swappgsout);
            swapfree = (swapin + swapout);// >> 10;
         }
         swappgsin  = vm_stats.pageins;
         swappgsout = vm_stats.pageouts;
      }
      else
      {
         AFERRPE("GetResources: host_satistics( HOST_VM_INFO) failure:");
      }

      if( getConstants )
      {
         host.mem_mb  = memtotal >> 10;
         host.swap_mb = 0;
      }
      hres.mem_free_mb    = ( memfree + memcached + membuffers ) >> 10;
      hres.mem_cached_mb  = memcached  >> 10;
      hres.mem_buffers_mb = membuffers >> 10;
      hres.swap_used_mb   = swapfree / af::Environment::getRenderUpdateSec();
   }//memory

   //
   // CPU usage:
   //
   {
      res * rl = &r0; res * rn = &r1;
      if( now ) {     rl = &r1;       rn = &r0; }

      unsigned int count = HOST_CPU_LOAD_INFO_COUNT;

      if( host_statistics( mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuload, &count) == KERN_SUCCESS)
      {
         rn->user   = (unsigned long) (cpuload.cpu_ticks[CPU_STATE_USER]);
         rn->nice   = (unsigned long) (cpuload.cpu_ticks[CPU_STATE_NICE]);
         rn->system = (unsigned long) (cpuload.cpu_ticks[CPU_STATE_SYSTEM]);
         rn->idle   = (unsigned long) (cpuload.cpu_ticks[CPU_STATE_IDLE]);

         int user    = rl->user     - rn->user;
         int nice    = rl->nice     - rn->nice;
         int system  = rl->system   - rn->system;
         int idle    = rl->idle     - rn->idle;
         int iowait  = rl->iowait   - rn->iowait;
         int irq     = rl->irq      - rn->irq;
         int softirq = rl->softirq  - rn->softirq;
         int total = user + nice + system + idle + iowait + irq + softirq;
         if( total == 0 ) total = 1;

         hres.cpu_user    = ( 100 * user    ) / total;
         hres.cpu_nice    = ( 100 * nice    ) / total;
         hres.cpu_system  = ( 100 * system  ) / total;
         hres.cpu_idle    = ( 100 * idle    ) / total;
         hres.cpu_iowait  = ( 100 * iowait  ) / total;
         hres.cpu_irq     = ( 100 * irq     ) / total;
         hres.cpu_softirq = ( 100 * softirq ) / total;
      }
      else
      {
         AFERRPE("GetResources: host_satistics( HOST_CPU_LOAD_INFO) failure:");
      }
      // CPU Load Average:
      double loadavg[3] = { 0, 0, 0 };
      int nelem = getloadavg( loadavg, 3);
      if( nelem < 2 ) loadavg[1] = loadavg[0];
      if( nelem < 3 ) loadavg[2] = loadavg[1];
      if( loadavg[0] > 25.0 ) loadavg[0] = 25.0;
      if( loadavg[1] > 25.0 ) loadavg[1] = 25.0;
      if( loadavg[2] > 25.0 ) loadavg[2] = 25.0;
      hres.cpu_loadavg1 = 10.0 * loadavg[0];
      hres.cpu_loadavg2 = 10.0 * loadavg[1];
      hres.cpu_loadavg3 = 10.0 * loadavg[2];
   }//cpu

   //
   // HDD space:
   //
   {
      static char path[4096];
      if( getConstants )
      {
         sprintf( path, "%s", af::Environment::getRenderHDDSpacePath().c_str());
         printf("HDD Space Path = '%s'\n", path);
      }
      struct statfs fsd;
      if( statfs( path, &fsd) < 0)
      {
         perror( "fs status:");
      }
      else
      {
         if( getConstants) host.hdd_gb = ((fsd.f_blocks >> 10) * fsd.f_bsize) >> 20;
         hres.hdd_free_gb  = ((fsd.f_bfree  >> 10) * fsd.f_bsize) >> 20;
      }
   }//hdd

   /*
      Network.

      We sum the statistics of all AF_LINK interfaces.
   */

   hres.net_recv_kbsec = 0;
   hres.net_send_kbsec = 0;

   {
      net *nl = &network_statistics[now];
      net *nn = &network_statistics[1-now];

      struct ifaddrs *addr;
      if( getifaddrs( &addr ) != 0 )
         return;

      unsigned net_recv = 0,  net_send = 0;

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

      nn->recv = net_recv;
      nn->send = net_send;
      net_recv = nn->recv - nl->recv;
      net_send = nn->send - nl->send;

      if( net_recv >= 0 )
      {
         hres.net_recv_kbsec = net_recv / af::Environment::getRenderUpdateSec();
         hres.net_recv_kbsec /= 1024;
      }

      if( net_send >= 0 )
      {
         hres.net_send_kbsec = net_send / af::Environment::getRenderUpdateSec();
         hres.net_send_kbsec /= 1024;
      }

      now = 1 - now;
   }
}
#endif
