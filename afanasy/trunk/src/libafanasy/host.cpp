#include "host.h"

#include "msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

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

Host::~Host(){}

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
   printf("Host: ");
   if( full)
   {
      printf("CPU = %d MHz x %d, MEM = %d (+%d swap) Mb, HDD = %d Gb\n",
         cpu_mhz, cpu_num, mem_mb, swap_mb, hdd_gb);
      printf("   OS=\"%s\": Capacity = %d, Max tasks = %d, Power = %d\n",
         os.toUtf8().data(), capacity, maxtasks, power);
      for( int i = 0; i < servicesnum; i++)
      {
         printf("   Service: \"%s\" - %d\n", servicesnames[i].toUtf8().data(), servicescounts[i]);
      }
      if( servicesnum == 0) printf("   No services descripion.\n");
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


HostResMeter::HostResMeter(){}
HostResMeter::HostResMeter( Msg * msg){ read( msg);}

void HostResMeter::readwrite( Msg * msg)
{
   rw_int32_t( value,      msg);
   rw_int32_t( valuemax,   msg);
   rw_uint8_t( width,      msg);
   rw_uint8_t( height,     msg);
   rw_uint8_t( graphr,     msg);
   rw_uint8_t( graphg,     msg);
   rw_uint8_t( graphb,     msg);
   rw_uint8_t( labelsize,  msg);
   rw_uint8_t( labelr,     msg);
   rw_uint8_t( labelg,     msg);
   rw_uint8_t( labelb,     msg);
   rw_uint8_t( bgcolorr,   msg);
   rw_uint8_t( bgcolorg,   msg);
   rw_uint8_t( bgcolorb,   msg);
   rw_QString( label,      msg);
   rw_QString( tooltip,    msg);
}

void HostResMeter::stdOut( bool full) const
{
   printf("%s: %d of %d (wh(%d,%d) gc(%d,%d,%d) ls%d lc(%d,%d,%d) bgc(%d,%d,%d))\n%s\n",
      label.toUtf8().data(), value, valuemax, width, height,
      graphr, graphg, graphb, labelsize, labelr, labelg, labelb, bgcolorr, bgcolorg, bgcolorb,
      tooltip.toUtf8().data());
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
   hdd_rd_kbsec(0),
   hdd_wr_kbsec(0),
   hdd_busy(0),

   net_recv_kbsec(0),
   net_send_kbsec(0)
{
}

HostRes::~HostRes()
{
   for( unsigned i = 0; i < custom.size(); i++) if( custom[i]) delete custom[i];
}

void HostRes::copy( const HostRes & other)
{
   cpu_loadavg1     = other.cpu_loadavg1;
   cpu_loadavg2     = other.cpu_loadavg2;
   cpu_loadavg3     = other.cpu_loadavg3;
   cpu_user         = other.cpu_user;
   cpu_nice         = other.cpu_nice;
   cpu_system       = other.cpu_system;
   cpu_idle         = other.cpu_idle;
   cpu_iowait       = other.cpu_iowait;
   cpu_irq          = other.cpu_irq;
   cpu_softirq      = other.cpu_softirq;
   mem_free_mb      = other.mem_free_mb;
   mem_cached_mb    = other.mem_cached_mb;
   mem_buffers_mb   = other.mem_buffers_mb;
   swap_used_mb     = other.swap_used_mb;
   hdd_free_gb      = other.hdd_free_gb;
   hdd_rd_kbsec     = other.hdd_rd_kbsec;
   hdd_wr_kbsec     = other.hdd_wr_kbsec;
   hdd_busy         = other.hdd_busy;
   net_recv_kbsec   = other.net_recv_kbsec;
   net_send_kbsec   = other.net_send_kbsec;

   if( custom.size() != other.custom.size())
   {
      for( unsigned i = 0; i < custom.size(); i++)
         if( custom[i] ) delete custom[i];
      custom.clear();
      for( unsigned i = 0; i < other.custom.size(); i++)
         custom.push_back( new HostResMeter());
   }

   for( unsigned i = 0; i < custom.size(); i++)
      *(custom[i]) = *(other.custom[i]);
}

void HostRes::readwrite( Msg * msg)
{
   rw_uint8_t( cpu_loadavg1,     msg);
   rw_uint8_t( cpu_loadavg2,     msg);
   rw_uint8_t( cpu_loadavg3,     msg);
   rw_uint8_t( cpu_user,         msg);
   rw_uint8_t( cpu_nice,         msg);
   rw_uint8_t( cpu_system,       msg);
   rw_uint8_t( cpu_idle,         msg);
   rw_uint8_t( cpu_iowait,       msg);
   rw_uint8_t( cpu_irq,          msg);
   rw_uint8_t( cpu_softirq,      msg);
   rw_int32_t( mem_free_mb,      msg);
   rw_int32_t( mem_cached_mb,    msg);
   rw_int32_t( mem_buffers_mb,   msg);
   rw_int32_t( swap_used_mb,     msg);
   rw_int32_t( hdd_free_gb,      msg);
   rw_int32_t( hdd_rd_kbsec,     msg);
   rw_int32_t( hdd_wr_kbsec,     msg);
   rw_int8_t ( hdd_busy,         msg);
   rw_int32_t( net_recv_kbsec,   msg);
   rw_int32_t( net_send_kbsec,   msg);

   uint8_t custom_count = custom.size();

   if( msg->isReading())
   {
      for( int i = 0; i < custom_count; i++) delete custom[i];
      custom.clear();
   }

   rw_uint8_t( custom_count,     msg);

   for( int i = 0; i < custom_count; i++)
      if( msg->isWriting()) custom[i]->write( msg);
      else custom.push_back( new HostResMeter( msg));
}

void HostRes::stdOut( bool full) const
{
   printf("Resources: ");
   if( full)
   {
      printf("Load average: %g %g %g\n", cpu_loadavg1/10.0, cpu_loadavg2/10.0, cpu_loadavg3/10.0);
      printf("   CPU usage: %d%% usr, %d%% nice, %d%% sys, %d%% idle, %d%% iow, %d%% irq, %d%% sirq\n",
         cpu_user, cpu_nice, cpu_system, cpu_idle, cpu_iowait, cpu_irq, cpu_softirq);
      printf("   Memory: Free %d Mb, Cached %d Mb, Buffers %d Mb, Swap %d Mb and HDD Free %d Gb\n",
         mem_free_mb, mem_cached_mb, mem_buffers_mb, swap_used_mb, hdd_free_gb );
      printf("   Network: Recieved %d Kb/sec, Send %d Kb/sec\n",
         net_recv_kbsec, net_send_kbsec );
      printf("   IO: Read %d Kb/sec, Write %d Kb/sec, Busy = %d%%\n",
         hdd_rd_kbsec, hdd_wr_kbsec, hdd_busy);
   }
   else
   {
      printf("la[%g,%g,%g]", cpu_loadavg1/10.0, cpu_loadavg2/10.0, cpu_loadavg3/10.0);
      printf("; C: u%d%% n%d%% s%d%% i%d%% o%d%% r%d%% f%d%%",
         cpu_user, cpu_nice, cpu_system, cpu_idle, cpu_iowait, cpu_irq, cpu_softirq);
      printf("; M: F%d C%d B%d, S%d; H%d; N%d/%d; D%d\%d:%d%%\n",
         mem_free_mb, mem_cached_mb, mem_buffers_mb, swap_used_mb, hdd_free_gb,
         net_recv_kbsec, net_send_kbsec,
         hdd_rd_kbsec, hdd_wr_kbsec, hdd_busy);
   }

   for( unsigned i = 0; i < custom.size(); i++ ) custom[i]->stdOut( full);
}
