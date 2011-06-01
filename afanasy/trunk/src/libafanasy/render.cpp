#include "render.h"

#include "address.h"
#include "environment.h"
#include "msg.h"
#include "taskexec.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

Render::Render( uint32_t State, uint8_t Priority):
   Client( Client::GetEnvironment, 0),
   state( State)
{
   construct();
   priority = Priority;
}

Render::Render( int Id):
   Client( Client::DoNotGetAnyValues, Id)
{
   construct();
}

Render::Render( Msg * msg):
   Client( Client::DoNotGetAnyValues, 0)
{
   construct();
   read( msg);
}

void Render::construct()
{
   maxrunningtasks = -1;
   capacity = -1;
   capacity_used = 0;
   wol_operation_time = 0;
}

Render::~Render()
{
}

void Render::readwrite( Msg * msg)
{
   switch( msg->type())
   {
   case Msg::TRendersList:

      rw_bool   ( locked,              msg);
      rw_int64_t( taskstartfinishtime, msg);
      rw_int32_t( maxrunningtasks,     msg);
      rw_int32_t( capacity,            msg);
      rw_int32_t( capacity_used,       msg);
      rw_int64_t( time_update,         msg);
      rw_int64_t( time_register,       msg);
      rw_int64_t( wol_operation_time,  msg);
      rw_String ( annotation,          msg);

      if( msg->isWriting())
      {
         uint32_t taskscount = uint32_t(tasks.size());
         rw_uint32_t( taskscount, msg);
         std::list<TaskExec*>::iterator it = tasks.begin();
         for( unsigned t = 0; t < taskscount; t++) (*(it++))->write( msg);
      }
      else
      {
         uint32_t taskscount;
         rw_uint32_t( taskscount, msg);
         for( unsigned t = 0; t < taskscount; t++) tasks.push_back( new TaskExec( msg));
      }

   case Msg::TRenderRegister:

      rw_int32_t ( revision,              msg);
      rw_String  ( version,               msg);
      rw_String  ( name,                  msg);
      rw_String  ( username,              msg);
      rw_uint32_t( state,                 msg);
      rw_uint32_t( flags,                 msg);
      rw_uint8_t ( priority,              msg);
      rw_int64_t ( time_launch,           msg);
      host.readwrite( msg);
      address.readwrite( msg);

   case Msg::TRenderUpdate:
   case Msg::TRendersListUpdates:

      hres.readwrite( msg);

      break;
   default:
      AFERROR("Render::readwrite(): invalid type.\n");
   }

   // Always send ID
   rw_int32_t( id, msg);

   // Send network interfaces information only when register
   if( msg->type() == Msg::TRenderRegister)
   {
      int8_t netIfs_size = netIFs.size();
      rw_int8_t( netIfs_size, msg);
      for( int i = 0; i < netIfs_size; i++)
         if( msg->isWriting()) netIFs[i]->write( msg);
         else netIFs.push_back( new NetIF( msg));
   }
}

void Render::setCapacity( int value)
{
   capacity = value;
   checkDirty();
}

void Render::checkDirty()
{
   if( capacity == host.capacity ) capacity = -1;
   if(( capacity == -1 ) && ( services_disabled.empty() ))
      state = state | SDirty;
   else
      state = state & (~SDirty);
}

void Render::restoreDefaults()
{
   capacity = host.capacity;
   services_disabled.clear();
   state = state & (~SDirty);
}

int Render::calcWeight() const
{
   int weight = Client::calcWeight();
   weight += sizeof(Render) - sizeof( Client);
   for( std::list<TaskExec*>::const_iterator it = tasks.begin(); it != tasks.end(); it++) weight += (*it)->calcWeight();
   return weight;
}

const std::string Render::getResourcesString() const
{
   std::ostringstream stream;
   stream << "Resources:";
   stream << "\n   CPU usage: "
         << unsigned( hres.cpu_idle    ) << "% idle, "
         << unsigned( hres.cpu_user    ) << "% user, "
         << unsigned( hres.cpu_nice    ) << "% user nice, "
         << unsigned( hres.cpu_system  ) << "% system, "
         << unsigned( hres.cpu_iowait  ) << "% i/o, "
         << unsigned( hres.cpu_irq     ) << "% irq, "
         << unsigned( hres.cpu_softirq ) << "% soft irq";
   stream << "\n   Memory:"
         << " free " << hres.mem_free_mb << " Mb of total " << host.mem_mb << " Mb"
         << ": unused " << hres.mem_free_mb << " Mb"
         << ", cached " << hres.mem_cached_mb << " Mb"
         << ", buffers " << hres.mem_buffers_mb << " Mb";
   stream << "\n   Swap: used " << hres.swap_used_mb << " Mb of total " << host.swap_mb << " Mb";
   stream << "\n   Network: recieving " << hres.net_recv_kbsec << " Kb/sec, sending " << hres.net_send_kbsec << " Kb/sec";
   stream << "\n   HDD: free " << hres.hdd_free_gb << " Gb of total " << host.hdd_gb << " Gb";

   return stream.str();
}

void Render::generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( full)
   {
      stream << "Render " << name << "@" << username << " (id=" << id << "):";
      stream << "\n Version = \"" << version << "\" Build Revision = " << revision;

      if( isDirty()) stream << "\nDirty! Capacity|Max Tasks changed, or service(s) disabled.";

      stream << std::endl;
      address.generateInfoStream( stream ,full);

      stream << "\n Priority = " << int(priority);
      stream << "\n Capacity = " << getCapacityFree() << " of " << getCapacity() << " ( " << getCapacityUsed() << " used )";

      stream << "\n Status:";
      if( isOnline()) stream << " Online";
      if( isOffline()) stream << " Offline";
//      if( isFree()) stream << " Free";
      if( isBusy()) stream << " Busy";
      if( isNimby()) stream << " (nimby)";
      if( isNIMBY()) stream << " (NIMBY)";
      if( isWOLFalling()) stream << " WOL-Falling";
      if( isWOLSleeping()) stream << " WOL-Sleeping";
      if( isWOLWaking()) stream << " WOL-Waking";

      if( wol_operation_time ) stream << "\n WOL operation time = " << time2str( wol_operation_time);
      if( time_launch   ) stream << "\n Launched at: " << time2str( time_launch   );
      if( time_register ) stream << "\n Registered at: " << time2str( time_register );

      stream << std::endl;
      host.generateInfoStream( stream, full);

      if( netIFs.size())
      {
         stream << "\nNetwork Interfaces:";
         for( int i = 0; i < netIFs.size(); i++)
         {
            stream << "\n   ";
            netIFs[i]->generateInfoStream( stream, true);
         }
      }

//      hres.generateInfoStream( stream ,full);
   }
   else
   {
      stream << name << "@" << username << "[" << id << "]";
      stream << " (" << version << " r" << revision << ") ";
      address.generateInfoStream( stream ,full);
//      stream << " - " << calcWeight() << " bytes.";
   }
}
