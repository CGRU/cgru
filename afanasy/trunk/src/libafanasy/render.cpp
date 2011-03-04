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

Render::Render( Msg * msg, const af::Address * addr):
   Client( Client::DoNotGetAnyValues, 0)
{
   construct();
   read( msg);
   if((addr != NULL) && (address != NULL)) address->setIP( addr);
}

void Render::construct()
{
   capacity = -1;
   capacity_used = 0;
}

Render::~Render()
{
}

void Render::readwrite( Msg * msg)
{
   switch( msg->type())
   {
   case Msg::TRendersList:

      rw_bool    ( locked,                msg);
      rw_uint32_t( taskstartfinishtime,   msg);
      rw_int32_t ( capacity,              msg);
      rw_int32_t ( capacity_used,         msg);
      rw_uint32_t( time_update,           msg);
      rw_uint32_t( time_register,         msg);
      rw_String (  annotation,            msg);

      if( msg->isWriting())
      {
         uint32_t taskscount = tasks.size();
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
      rw_uint8_t ( priority,              msg);
      rw_uint32_t( time_launch,           msg);
      host.readwrite( msg);
      if( isOnline())
      {
         if( msg->isWriting())
         {
            if( address != NULL ) address->write( msg);
            else
               AFERRAR("Render::readwrite: Trying to write online \"%s\" Render with NULL address.\n", name.c_str());
         }
         else
         {
            if( address ) delete address;
            address = new Address( msg);
         }
      }

   case Msg::TRenderUpdate:
   case Msg::TRendersListUpdates:

      hres.readwrite( msg);

      break;
   default:
      AFERROR("Render::readwrite(): invalid type.\n");
   }

   rw_int32_t( id, msg);
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

bool Render::addTask( TaskExec * taskexec)
{
   bool becamebusy = false;
   if( tasks.size() == 0)
   {
      setBusy( true);
      becamebusy = true;
      taskstartfinishtime = time( NULL);
   }
   tasks.push_back( taskexec);

   capacity_used += taskexec->getCapResult();

   if( capacity_used > getCapacity() )
      AFERRAR("Render::addTask(): capacity_used > host.capacity (%d>%d)\n", capacity_used, host.capacity);

   return becamebusy;
}

bool Render::removeTask( const TaskExec * taskexec)
{
   bool becamefree = false;
   for( std::list<TaskExec*>::iterator it = tasks.begin(); it != tasks.end(); it++)
   {
      if( *it == taskexec)
      {
         it = tasks.erase( it);
         continue;
      }
   }

   if( capacity_used < taskexec->getCapResult())
   {
      AFERRAR("Render::removeTask(): capacity_used < taskdata->getCapResult() (%d<%d)\n", capacity_used, taskexec->getCapResult());
      capacity_used = 0;
   }
   else capacity_used -= taskexec->getCapResult();

   if( tasks.size() == 0)
   {
      setBusy( false);
      becamefree = true;
      taskstartfinishtime = time( NULL);
   }

   return becamefree;
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
         << " free " << (hres.mem_free_mb + hres.mem_cached_mb + hres.mem_buffers_mb) << " Mb of total " << host.mem_mb << " Mb"
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
      stream << "#" << id << ": " << name << "@" << username;
      stream << " (" << version << " r" << revision << ")";
      stream << " :: ";
      if( address == NULL) stream << "address == NULL";
      else address->generateInfoStream( stream ,full);
      stream << " - " << calcWeight() << " bytes.";

      stream << std::endl;

      host.generateInfoStream( stream ,full);
      hres.generateInfoStream( stream ,full);
      stream << "\n Ready = " << ( isReady() ? "TRUE" : "FALSE" );
      stream << "\n Busy  = " << ( isBusy()  ? "true" : "false" );
      stream << "\n NIMBY = " << ( isNIMBY() ? "true" : "false" );
      stream << "\n Nimby = " << ( isNimby() ? "true" : "false" );
      if( time_launch   ) stream << "\n Launched at   = " << time2str( time_launch   );
      if( time_register ) stream << "\n Registered at = " << time2str( time_register );
   }
   else
   {
      stream << name << "@" << username << "[" << id << "]";
      stream << " (" << version << " r" << revision << ") ";
      if( address == NULL) stream << "address == NULL";
      else address->generateInfoStream( stream ,full);
//      stream << " - " << calcWeight() << " bytes.";
   }
}
