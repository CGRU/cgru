#include "monitor.h"

#include "environment.h"
#include "address.h"
#include "msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

const int Monitor::EventsShift = af::Msg::TMonitorEvents_BEGIN + 1;
const int Monitor::EventsCount = af::Msg::TMonitorEvents_END - Monitor::EventsShift;

Monitor::Monitor():
   Client( Client::GetEnvironment, 0)
{
   construct();
   name = QString("%1@%2:%3").arg(af::Environment::getUserName(), af::Environment::getHostName()).arg( address->getPortString());
}

Monitor::Monitor( Msg * msg, const Address * addr):
   Client( Client::DoNotGetAnyValues, 0)
{
   if( construct() == false) return;
   read( msg);
   if( addr != NULL) address->setIP( addr);
}

bool Monitor::construct()
{
   events = new bool[EventsCount];
   if( events == NULL)
   {
      AFERROR("Monitor::construct(): Can't allocate memory for events table.\n");
      return false;
   }
   for( int e = 0; e < EventsCount; e++) events[e] = false;
   return true;
}

Monitor::~Monitor()
{
   if( events ) delete [] events;
}

void Monitor::readwrite( Msg * msg)
{
   rw_int32_t ( id,            msg);
   rw_uint32_t( time_launch,   msg);
   rw_uint32_t( time_register, msg);
   rw_uint32_t( time_activity, msg);
   rw_QString ( name,          msg);
   rw_QString ( username,      msg);
   rw_int32_t(  revision,      msg);
   rw_QString(  version,       msg);

   for( int e = 0; e < EventsCount; e++) rw_bool( events[e], msg);

   rw_Int32_List( jobsUsersIds, msg);
   rw_Int32_List( jobsIds,      msg);

   if( msg->isWriting() ) address->write( msg);
   else address = new Address( msg);
}

bool Monitor::hasEvent( int type) const
{
   int eventNum = type - EventsShift;
   if((eventNum >= 0) && (eventNum < EventsCount))
   {
      return events[eventNum];
   }
   else
   {
      AFERRAR("MonitorAf::hasEvent: Invalid event: [%s]\n", af::Msg::TNAMES[type]);
      return false;
   }
}

void Monitor::stdOut( bool full) const
{
   printf("#%d: %s@%s :: ", id, name.toUtf8().data(), username.toUtf8().data());
   address->stdOut();
   printf("\n");
   printf("   Version: %s rev%d\n", version.toUtf8().data(), revision);

   printf(" UIds[%d]:", (int)(jobsUsersIds.size()));
   for( std::list<int32_t>::const_iterator it = jobsUsersIds.begin(); it != jobsUsersIds.end(); it++)
      printf(" %d", *it);
   printf("; JIds[%d]:", (int)(jobsIds.size()));
   for( std::list<int32_t>::const_iterator it = jobsIds.begin(); it != jobsIds.end(); it++)
      printf(" %d", *it);
   printf("\n");

   if( full == false ) return;
   for( int e = 0; e < EventsCount; e++)
   {
      if( full )
         printf("\t%s\n", events[e] ? Msg::TNAMES[e+EventsShift] : "-");
      else
         printf(" %s",    events[e] ? Msg::TNAMES[e+EventsShift] : "-");
   }
   printf("\n");
}
