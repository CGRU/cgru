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

const char netstat[] = "netstat -i";
const int buffer_len = 1024;
char buffer[buffer_len];
char find_net[] = "Address";
const int find_net_len = 7;
char iname[buffer_len];
int  iname_len = 0;

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
}n0,n1;

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
   char path[] = "/";
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


   //
   // Network:
   //
   {
             net * nl = &n0; net * nn = &n1;
   if( now ) {     nl = &n1;       nn = &n0; }
   bool verbose = false;

   int net_recv_kb = 0; int net_send_kb = 0;
   FILE *f = NULL;
   f = popen( netstat, "r");
   if( fgets( buffer, buffer_len, f) != NULL )
   {
      static int start;
      if( getConstants )
      {
         start = 0;
	     while( strncmp( buffer+(start++), find_net, find_net_len) != 0);
	     start += find_net_len;
         if( verbose) printf("start=%d\n", start);
      }
      while( fgets( buffer, buffer_len, f))
      {
         int pos = 0;
         if( iname_len && (strncmp( buffer, iname, iname_len) == 0)) continue;

	     while( buffer[++pos] != ' ');
		 iname_len = pos;
		 strncpy( iname, buffer, iname_len);
		 iname[iname_len+1] = '\0';

         while( buffer[++pos] == ' ');
		 unsigned int mtu = atoi( buffer+pos);
         pos = start;
         while( buffer[++pos] != ' '); while( buffer[++pos] == ' ');
         unsigned int Ipkts = atoi( buffer+pos);
         while( buffer[++pos] != ' '); while( buffer[++pos] == ' ');
         while( buffer[++pos] != ' '); while( buffer[++pos] == ' ');
		 unsigned int Opkts = atoi( buffer+pos);
         net_recv_kb += (Ipkts * mtu) >> 10;
         net_send_kb += (Opkts * mtu) >> 10;
      }
   }
   pclose( f);
   nn->recv = net_recv_kb;
   nn->send = net_send_kb;
   net_recv_kb = nn->recv - nl->recv;
   net_send_kb = nn->send - nl->send;
   if( verbose) printf("net_recv_kb=%d net_send_kb=%d\n", net_recv_kb, net_send_kb);
   if( net_recv_kb >= 0 ) hres.net_recv_kbsec = net_recv_kb / af::Environment::getRenderUpdateSec();
   if( net_send_kb >= 0 ) hres.net_send_kbsec = net_send_kb / af::Environment::getRenderUpdateSec();
   }//network

   now = 1 - now;

//hres.stdOut(false);
}
#endif
