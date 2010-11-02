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
      rw_QString ( annotation,            msg);

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

      rw_QString ( name,                  msg);
      rw_QString ( username,              msg);
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
               AFERRAR("Render::readwrite: Trying to write online \"%s\" Render with NULL address.\n", name.toUtf8().data());
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
   if(( capacity == -1) && ( services_disabled.isEmpty()))
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

const QString Render::getResources() const
{
   QString str = "Resources:";
   str += QString("\n   CPU usage: %1% idle, %2% user, %3% user nice, %4% system, %5% i/o, %6% irq, %7% soft irq")
         .arg(hres.cpu_idle).arg(hres.cpu_user).arg(hres.cpu_nice).arg(hres.cpu_system).arg(hres.cpu_iowait).arg(hres.cpu_irq).arg(hres.cpu_softirq);
   str += QString("\n   Memory: free %1 Mb of total %2 Mb: unused %3 Mb, cached %4 Mb, buffers %5 Mb")
         .arg(hres.mem_free_mb + hres.mem_cached_mb + hres.mem_buffers_mb).arg(host.mem_mb)
         .arg(hres.mem_free_mb).arg(hres.mem_cached_mb).arg(hres.mem_buffers_mb);
   str += QString("\n   Swap: used %1 Mb of total %2 Mb").arg(hres.swap_used_mb).arg(host.swap_mb );
   str += QString("\n   Network: recieving %1 Kb/sec, sending %2 Kb/sec").arg(hres.net_recv_kbsec).arg(hres.net_send_kbsec );
   str += QString("\n   HDD: free %1 Gb of total %2 Gb").arg(hres.hdd_free_gb).arg(host.hdd_gb );
   return str;
}

void Render::stdOut( bool full) const
{
   printf("#%d:%d %s@%s :: ", id, priority, name.toUtf8().data(), username.toUtf8().data());
   if( address == NULL) printf("address == NULL");
   else address->stdOut();
   printf(" - %d bytes.\n", calcWeight());

   host.stdOut( full);
   hres.stdOut( full);
   if( full == false ) return;

   printf("   Ready = %s\n", isReady() ? "TRUE" : "FALSE");
   printf("   Busy  = %s\n", isBusy()  ? "true" : "false");
   printf("   NIMBY = %s\n", isNIMBY() ? "true" : "false");
   printf("   Nimby = %s\n", isNimby() ? "true" : "false");
   if( time_launch   ) printf("   Launched at   = %s\n", time2Qstr( time_launch   ).toUtf8().data());
   if( time_register ) printf("   Registered at = %s\n", time2Qstr( time_register ).toUtf8().data());
}
