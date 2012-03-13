#include "host.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

void HostRes::readwrite( Msg * msg)
{
      rw_uint8_t ( cpu_loadavg1,     msg);
      rw_uint8_t ( cpu_loadavg2,     msg);
      rw_uint8_t ( cpu_loadavg3,     msg);
      rw_uint8_t ( cpu_user,         msg);
      rw_uint8_t ( cpu_nice,         msg);
      rw_uint8_t ( cpu_system,       msg);
      rw_uint8_t ( cpu_idle,         msg);
      rw_uint8_t ( cpu_iowait,       msg);
      rw_uint8_t ( cpu_irq,          msg);
      rw_uint8_t ( cpu_softirq,      msg);
      rw_int32_t ( mem_free_mb,      msg);
      rw_int32_t ( mem_cached_mb,    msg);
      rw_int32_t ( mem_buffers_mb,   msg);
      rw_int32_t ( swap_used_mb,     msg);
      rw_int32_t ( hdd_free_gb,      msg);
      rw_uint32_t( net_recv_kbsec,   msg);
      rw_uint32_t( net_send_kbsec,   msg);
}

Host::Host():
   capacity( 0),
   maxtasks( 0),
   power(0),
   cpu_num(0),
   cpu_mhz(0),
   mem_mb(0),
   swap_mb(0),
   hdd_gb(0),
   os(""),
   properties(""),
   servicesnum(0)
{
}

HostRes::HostRes():
   cpu_loadavg1(0),
   cpu_loadavg2(0),
   cpu_loadavg3(0),

   cpu_user(0),
   cpu_nice(0),
   cpu_system(0),
   cpu_idle(99),
   cpu_iowait(0),
   cpu_irq(0),
   cpu_softirq(0),


   mem_free_mb(1),
   mem_cached_mb(0),
   mem_buffers_mb(0),

   swap_used_mb(0),

   hdd_free_gb(1),

   net_recv_kbsec(0),
   net_send_kbsec(0)
{
}

Host::~Host()
{
}

void Host::setService( const QString & name, int count)
{
   int index = servicesnames.indexOf( name);
   if( index == -1)
   {
      servicesnames.append(name);
      servicescounts.push_back( count);
      servicesnum++;
   }
   else
      servicescounts[index] = count;
}

void Host::copy( const Host & other)
{
//printf("Host::copy: servicesnum = %d\n", servicesnum);
   if( other.maxtasks      ) maxtasks = other.maxtasks;
   if( other.capacity      ) capacity = other.capacity;
   if( other.power         ) power    = other.power;
   if( other.cpu_num       ) cpu_num  = other.cpu_num;
   if( other.cpu_mhz       ) cpu_mhz  = other.cpu_mhz;
   if( other.mem_mb        ) mem_mb   = other.mem_mb;
   if( other.hdd_gb        ) hdd_gb   = other.hdd_gb;
   if( other.swap_mb       ) swap_mb  = other.swap_mb;

   if(!other.os.isEmpty()           ) os           = other.os;
   if(!other.properties.isEmpty()   ) properties   = other.properties;
   if(!other.resources.isEmpty()    ) resources    = other.resources;
   if(!other.data.isEmpty()         ) data         = other.data;

   servicesnames.clear();
   servicescounts.clear();
   servicesnum = 0;

   for( int i = 0; i < other.servicesnum; i++) setService( other.servicesnames[i], other.servicescounts[i]);
//printf("Host::copy: servicesnum = %d\n", servicesnum);
}

void Host::readwrite( Msg * msg)
{
   rw_int32_t( maxtasks,     msg);
   rw_int32_t( capacity,     msg);
   rw_int32_t( power,        msg);
   rw_int32_t( cpu_num,      msg);
   rw_int32_t( cpu_mhz,      msg);
   rw_int32_t( mem_mb,       msg);
   rw_int32_t( swap_mb,      msg);
   rw_int32_t( hdd_gb,       msg);
   rw_QString( os,           msg);
   rw_QString( properties,   msg);
   rw_QString( resources,    msg);
   rw_QString( data,         msg);
}

void Host::stdOut( bool full) const
{
   if( full)
   {
      printf("OS=\"%s\": Capacity = %d, Max tasks = %d, Power = %d, CPU = %d MHz x %d, MEM = %d (+%d swap) Mb, HDD = %d Gb\n",
         os.toUtf8().data(), capacity, maxtasks, power, cpu_mhz, cpu_num, mem_mb, swap_mb, hdd_gb);
      for( int i = 0; i < servicesnum; i++)
      {
         printf("\tService: \"%s\" - %d\n", servicesnames[i].toUtf8().data(), servicescounts[i]);
      }
      if( servicesnum == 0) printf("\tNo services descripion.\n");
   }
   else
   {
      printf("OS=\"%s\": CAP%d MAX=%d P%d Cpu%dx%d M%d+%ds H%d",
         os.toUtf8().data(), capacity, maxtasks, power, cpu_mhz, cpu_num, mem_mb, swap_mb, hdd_gb);
      for( int i = 0; i < servicesnum; i++)
      {
         printf(", \"%s\"[%d]", servicesnames[i].toUtf8().data(), servicescounts[i]);
      }
      if( servicesnum == 0) printf(" No services.");
      printf("\n");
   }
}

void HostRes::stdOut( bool full) const
{
   if( full)
   {
      printf("Load average: %g %g %g\n", cpu_loadavg1/10.0, cpu_loadavg2/10.0, cpu_loadavg3/10.0);
      printf("CPU usage: %d%% usr, %d%% nice, %d%% sys, %d%% idle, %d%% iow, %d%% irq, %d%% sirq\n",
         cpu_user, cpu_nice, cpu_system, cpu_idle, cpu_iowait, cpu_irq, cpu_softirq);
      printf("Memory: Free %d Mb, Cached %d Mb, Buffers %d Mb, Swap %d Mb and HDD Free %d Gb\n",
         mem_free_mb, mem_cached_mb, mem_buffers_mb, swap_used_mb, hdd_free_gb );
      printf("Network: Recieved %d Kb/sec, Send %d Kb/sec\n",
         net_recv_kbsec, net_send_kbsec );
   }
   else
   {
      printf("la[%g,%g,%g]", cpu_loadavg1/10.0, cpu_loadavg2/10.0, cpu_loadavg3/10.0);
      printf("; C: u%d%% n%d%% s%d%% i%d%% o%d%% r%d%% f%d%%",
         cpu_user, cpu_nice, cpu_system, cpu_idle, cpu_iowait, cpu_irq, cpu_softirq);
      printf("; M: F%d C%d B%d, S%d; H%d; N%d/%d\n",
         mem_free_mb, mem_cached_mb, mem_buffers_mb, swap_used_mb, hdd_free_gb, net_recv_kbsec, net_send_kbsec );
   }
}
