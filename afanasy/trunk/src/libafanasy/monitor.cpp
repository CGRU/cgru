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
   name = af::Environment::getUserName() + "@" + af::Environment::getHostName() + ":" + address->generatePortString().c_str();
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
   rw_String  ( name,          msg);
   rw_String  ( username,      msg);
   rw_int32_t ( revision,      msg);
   rw_String  ( version,       msg);

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

void Monitor::generateInfoStream( std::ostringstream & stream, bool full) const
{
   stream << name << "[" << id << "]";
   stream << " (" << version << " r" << revision << ") ";
   address->generateInfoStream( stream);

   if( full == false ) return;

   stream << std::endl;
   stream << " UIds[" << jobsUsersIds.size() << "]:";
   for( std::list<int32_t>::const_iterator it = jobsUsersIds.begin(); it != jobsUsersIds.end(); it++)
      stream << " " << *it;
   stream << "; JIds[" << jobsIds.size() << "]:";
   for( std::list<int32_t>::const_iterator it = jobsIds.begin(); it != jobsIds.end(); it++)
      stream << " " << *it;
   stream << std::endl;
   for( int e = 0; e < EventsCount; e++)
      stream << "\t" << (events[e] ? Msg::TNAMES[e+EventsShift] : "-") << "\n";
}
