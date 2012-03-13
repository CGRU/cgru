#ifdef WINNT
#include "res.h"

#include <environment.h>

void GetResources( af::Host & host, af::HostRes & hres, bool getConstants)
{
   //
   // CPU info:
   //
   if( getConstants)
   {
      // number of pocessors
      host.cpu_num = 1;
      // frequency of first pocessor
      host.cpu_mhz = 1000;
   }

   //
   // Memory & Swap total and usage:
   //
   {
   if( getConstants )
   {
      host.mem_mb  = 1000;
      host.swap_mb = 1000;
   }
   hres.mem_free_mb    = 1000;
   hres.mem_cached_mb  = 0;
   hres.mem_buffers_mb = 0;
   hres.swap_used_mb   = 0;
   }//memory

   //
   // CPU usage:
   //
   {
   hres.cpu_user    = 0;
   hres.cpu_nice    = 0;
   hres.cpu_system  = 0;
   hres.cpu_idle    = 0;
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
   hres.cpu_loadavg1 = 10.0 * loadavg[0];
   hres.cpu_loadavg2 = 10.0 * loadavg[1];
   hres.cpu_loadavg3 = 10.0 * loadavg[2];
   }//cpu

   //
   // HDD space:
   //
   {
   if( getConstants) host.hdd_gb = 10;
   hres.hdd_free_gb  = 10;
   }//hdd

   //
   // Network:
   //
   {
   hres.net_recv_kbsec = 0;
   hres.net_send_kbsec = 0;
   }//network

//hres.stdOut(false);
}
#endif
