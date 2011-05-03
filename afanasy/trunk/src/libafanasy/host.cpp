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
   servicesnum(0),
   wol_idlesleep_time(0)
{
}

Host::~Host(){}

void Host::setService( const std::string & name, int count)
{
   int index = 0;
   bool exists = false;
   for( std::vector<std::string>::const_iterator it = servicesnames.begin(); it != servicesnames.end(); it++, index++)
      if( *it == name )
      {
         exists = true;
         break;
      }
   if( exists)
      servicescounts[index] = count;
   else
   {
      servicesnames.push_back(name);
      servicescounts.push_back( count);
      servicesnum++;
   }
}

void Host::clear()
{
   servicesnames.clear();
   servicescounts.clear();
   servicesnum = 0;
}

void Host::copy( const Host & other)
{
   clear();
   merge( other);
}

void Host::merge( const Host & other)
{
   mergeParameters( other);
   for( int i = 0; i < other.servicesnum; i++) setService( other.servicesnames[i], other.servicescounts[i]);
}

void Host::remServices( const std::list<std::string> & remNames)
{
   for( std::list<std::string>::const_iterator remIt = remNames.begin(); remIt != remNames.end(); remIt++)
   {
      std::vector<std::string>::iterator srvIt = servicesnames.begin();
      while( srvIt != servicesnames.end())
      {
         if( *srvIt == *remIt)
         {
            srvIt = servicesnames.erase( srvIt);
            servicesnum--;
         }
         else srvIt++;
      }
   }
}

void Host::mergeParameters( const Host & other)
{
   if( other.maxtasks      ) maxtasks = other.maxtasks;
   if( other.capacity      ) capacity = other.capacity;
   if( other.power         ) power    = other.power;
   if( other.cpu_num       ) cpu_num  = other.cpu_num;
   if( other.cpu_mhz       ) cpu_mhz  = other.cpu_mhz;
   if( other.mem_mb        ) mem_mb   = other.mem_mb;
   if( other.hdd_gb        ) hdd_gb   = other.hdd_gb;
   if( other.swap_mb       ) swap_mb  = other.swap_mb;

   if( other.os.size()        ) os           = other.os;
   if( other.properties.size()) properties   = other.properties;
   if( other.resources.size() ) resources    = other.resources;
   if( other.data.size()      ) data         = other.data;

   if( other.wol_idlesleep_time ) wol_idlesleep_time = other.wol_idlesleep_time;
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
   rw_String ( os,           msg);
   rw_String ( properties,   msg);
   rw_String ( resources,    msg);
   rw_String ( data,         msg);
}

void Host::generateInfoStream( std::ostringstream & stream, bool full) const
{
   stream << "Host:";
   if( full)
   {
      if( false == os.empty()) stream << " OS=\"" << os << "\":";
      stream << std::endl;
      stream << " Capacity = " << capacity;
      stream << ", Max tasks = " << maxtasks;
      stream << ", Power " << power;

      if( wol_idlesleep_time ) stream << "\n WOL Sleep Idle Time = " << time2strHMS( wol_idlesleep_time, true );

      if( cpu_mhz || cpu_num || mem_mb || swap_mb || hdd_gb )
      {
         stream << std::endl;
         stream << " CPU = " << cpu_mhz << " MHz" << " x" << cpu_num;
         stream << ", MEM = " << mem_mb << " (+" << swap_mb << " Swap) Mb";
         stream << ", HDD = " << hdd_gb << " Gb";
      }
   }
   else
   {
      if( false == os.empty()) stream << "OS=\"" << os << "\"";
      stream << " CAP" << capacity;
      stream << " MAX=" << maxtasks;
      stream << " P" << power;
      stream << " CPU" << cpu_mhz << "x" << cpu_num;
      stream << " M" << mem_mb << "+" << swap_mb << "S" << " H" << hdd_gb;
      stream << " WOL" << time2str( wol_idlesleep_time );
   }
}

void Host::generateServicesStream( std::ostringstream & stream) const
{
   for( int i = 0; i < servicesnum; i++)
   {
      if( i ) stream << std::endl;
      stream << "Service: \"" << servicesnames[i] << "\"";
      if( servicescounts[i]) stream << " - " << servicescounts[i];
   }
}

void Host::printServices() const
{
   std::ostringstream stream;
   generateServicesStream( stream);
   std::cout << stream << std::endl;
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
   rw_String(  label,      msg);
   rw_String(  tooltip,    msg);
}

void HostResMeter::generateInfoStream( std::ostringstream & stream, bool full) const
{
   stream << label   << ": ";
   stream << value   << " of " << valuemax;
   if( full)
   {
      stream << std::endl;
      stream << " WH(" << int(width)     << "," << int(height)   << ")";
      stream << " GC(" << int(graphr)    << "," << int(graphg)   << "," << int(graphb)   << ")";
      stream << " LS"  << int(labelsize);
      stream << " LC(" << int(labelr)    << "," << int(labelg)   << "," << int(labelb)   << ")";
      stream << " BC(" << int(bgcolorr)  << "," << int(bgcolorg) << "," << int(bgcolorb) << ")";
   }
   stream << std::endl;
   stream << tooltip;
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

   uint8_t custom_count = uint8_t(custom.size());

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

void HostRes::generateInfoStream( std::ostringstream & stream, bool full) const
{
   generateInfoStream( NULL, stream, full);
}

const std::string HostRes::generateInfoString( const Host * host, bool full) const
{
   std::ostringstream stream;
   generateInfoStream( host, stream, full);
   return stream.str();
}

void HostRes::generateInfoStream( const Host * host, std::ostringstream & stream, bool full) const
{
   stream << "Resources: ";
   if( full)
   {
      stream << "\n   CPU usage: "
            << int( cpu_user    ) << "% usr, "
            << int( cpu_nice    ) << "% nice, "
            << int( cpu_system  ) << "% sys, "
            << int( cpu_idle    ) << "% idle, "
            << int( cpu_iowait  ) << "% iow, "
            << int( cpu_irq     ) << "% irq, "
            << int( cpu_softirq ) << "% sirq";
      stream << "\n   Load average:   " << cpu_loadavg1/10.0 << "   " << cpu_loadavg2/10.0 << "   " << cpu_loadavg3/10.0;
      stream << "\n   Memory Free: " << mem_free_mb << " Mb";
      if( mem_cached_mb || mem_buffers_mb )
      {
         stream << " (Cache " << mem_cached_mb << " Mb";
         stream << ", Buffers " << mem_buffers_mb << " Mb)";
      }
      stream << std::endl;
      if( host) stream << "   Memory Used: " << host->mem_mb - mem_free_mb << " Mb,";
      stream << "   Swapped: " << swap_used_mb << " Mb";
      stream << "\n   Network: Recieved " << net_recv_kbsec << " Kb/sec, Send " << net_send_kbsec  << " Kb/sec",
      stream << "\n   IO: Read " << hdd_rd_kbsec << " Kb/sec, Write " << hdd_wr_kbsec << " Kb/sec, Busy = " << int(hdd_busy) << "%";
      stream << "\n   HDD: " << hdd_free_gb  << " Gb Free";
   }
   else
   {
      stream << "la[" << cpu_loadavg1/10.0 << "," << cpu_loadavg2/10.0 << "," << cpu_loadavg3/10.0 << "]";
      stream << "; C: u" << int(cpu_user)
            << "% n" << int(cpu_nice)
            << "% s" << int(cpu_system)
            << "% i" << int(cpu_idle)
            << "% o" << int(cpu_iowait)
            << "% r" << int(cpu_irq)
            << "% f" << int(cpu_softirq) << "%";
      stream << "; M: F" << mem_free_mb
            << " C" << mem_cached_mb
            << " B" << mem_buffers_mb
            << " S" << swap_used_mb
            << " H" << hdd_free_gb
            << " N" << net_recv_kbsec << "/" << net_send_kbsec
            << " D" << hdd_rd_kbsec << "\"" << hdd_wr_kbsec << ":" << int(hdd_busy) << "%";
   }

   for( unsigned i = 0; i < custom.size(); i++ )
   {
      stream << std::endl;
      custom[i]->generateInfoStream( stream, full);
   }
}
