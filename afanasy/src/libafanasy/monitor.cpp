#include "monitor.h"

#include "address.h"
#include "environment.h"
#include "msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

const int Monitor::EventsShift = af::Msg::TMonitorEvents_BEGIN + 1;
const int Monitor::EventsCount = af::Msg::TMonitorEvents_END - Monitor::EventsShift;

const char Monitor::EventsNames[Monitor::EventsCount][32] = {
	"JOB_BEGIN",
	"jobs_add",
	"jobs_change",
	"jobs_del",
	"JOB_END",
	"COMMON_BEGIN",
	"users_add",
	"users_change",
	"users_del",
	"renders_add",
	"renders_change",
	"renders_del",
	"monitors_add",
	"monitors_change",
	"monitors_del",
	"talks_add",
	"talks_del",
	"COMMON_END"
};
/*
const char Monitor::EventsClasses[Monitor::EventsCount][32] = {
	"jobs",
	"jobs",
	"jobs",
	"jobs",
	"jobs",
	"common",
	"users",
	"users",
	"users",
	"renders",
	"renders",
	"renders",
	"monitors",
	"monitors",
	"monitors",
	"talks",
	"talks",
	"common"
};
*/
Monitor::Monitor():
   Client( Client::GetEnvironment, 0)
//	m_listening_port(true)
{
   construct();
   m_name = af::Environment::getUserName() + "@" + af::Environment::getHostName() + ":" + m_address.generatePortString().c_str();
}

Monitor::Monitor( Msg * msg):
   Client( Client::DoNotGetAnyValues, 0)
//	m_listening_port(true)
{
   if( construct() == false) return;
   read( msg);
}

Monitor::Monitor( const JSON & obj):
   Client( Client::DoNotGetAnyValues, 0)
//	m_listening_port(false)
{
	m_time_launch = time(NULL);
	m_time_activity = 0;
	construct();
	jr_string("gui_name",  m_gui_name,  obj);
	jr_string("user_name", m_user_name, obj);
	jr_string("host_name", m_host_name, obj);
	jr_string("engine",    m_engine,    obj);

	m_name = m_user_name+"@"+m_host_name+":"+m_gui_name;

	m_version = af::Environment::getVersionCGRU();
}

bool Monitor::construct()
{
	m_uid = -1;
   events = new bool[EventsCount];
   if( events == NULL)
   {
      AFERROR("Monitor::construct(): Can't allocate memory for events table.")
      return false;
   }
   for( int e = 0; e < EventsCount; e++) events[e] = false;
   return true;
}

Monitor::~Monitor()
{
   if( events ) delete [] events;
}

void Monitor::v_readwrite( Msg * msg)
{
   rw_int32_t( m_id,            msg);
   rw_int64_t( m_time_launch,   msg);
   rw_int64_t( m_time_register, msg);
   rw_int64_t( m_time_activity, msg);
   rw_String ( m_name,          msg);
   rw_String ( m_user_name,     msg);
   rw_String ( m_version,       msg);

   for( int e = 0; e < EventsCount; e++) rw_bool( events[e], msg);

   rw_Int32_List( jobsUsersIds, msg);
   rw_Int32_List( jobsIds,      msg);

   m_address.v_readwrite( msg);
}

void Monitor::v_jsonWrite( std::ostringstream & o_str, int i_type) const
{
	Client::v_jsonWrite( o_str, i_type);

	o_str << ",\"uid\":" << m_uid;
	o_str << ",\"version\":\"" << m_version << "\"";
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
      AFERRAR("Monitor::hasEvent: Invalid event: [%s]", af::Msg::TNAMES[type])
      return false;
   }
}

void Monitor::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( full )
   {
      stream << "Monitor name = \"" << m_name << "\" (id=" << getId() << ")";
      stream << "\n Version: " << m_version;
      stream << "\n Launch Time: " + af::time2str( m_time_launch);
      stream << "\n Register Time: " + af::time2str( m_time_register);
		if( m_time_activity )
			stream << "\n Time Activity: " + af::time2str( m_time_activity);

      stream << "\n UIds[" << jobsUsersIds.size() << "]:";
      for( std::list<int32_t>::const_iterator it = jobsUsersIds.begin(); it != jobsUsersIds.end(); it++)
         stream << " " << *it;
      stream << "\n JIds[" << jobsIds.size() << "]:";
      for( std::list<int32_t>::const_iterator it = jobsIds.begin(); it != jobsIds.end(); it++)
         stream << " " << *it;

      stream << "\n System Events:";
      for( int e = 0; e < af::Monitor::EventsCount; e++)
      {
         int etype = e + af::Monitor::EventsShift;
         stream << "\n    " << af::Msg::TNAMES[etype] << " : ";
         if( hasEvent(etype)) stream << " SUBMITTED";
         else stream << "   ---";
      }
   }
   else
   {
      stream << m_name << "[" << m_id << "]";
      stream << " v'" << m_version << "' ";
      m_address.v_generateInfoStream( stream, full);
   }
}
