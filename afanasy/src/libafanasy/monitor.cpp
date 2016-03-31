#include "monitor.h"

#include "address.h"
#include "environment.h"
#include "msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

Monitor::Monitor():
   Client( Client::GetEnvironment, 0)
{
   construct();
   m_name = af::Environment::getUserName() + "@" + af::Environment::getHostName() + ":" + m_address.generatePortString().c_str();
}

Monitor::Monitor( Msg * msg):
   Client( Client::DoNotGetAnyValues, 0)
{
   if( construct() == false) return;
   read( msg);
}

Monitor::Monitor( const JSON & obj):
   Client( Client::DoNotGetAnyValues, 0)
{
	m_time_launch = time(NULL);
	m_time_activity = 0;
	construct();
	jr_string("user_name", m_user_name, obj);
	jr_string("host_name", m_host_name, obj);
	jr_string("engine",    m_engine,    obj);

	m_name = m_user_name + "@" + m_host_name;
}

bool Monitor::construct()
{
	m_uid = -1;

	m_events.resize( EVT_COUNT);
	for( int e = 0; e < m_events.size(); e++)
		m_events[e] = false;

	return true;
}

Monitor::~Monitor()
{
}

void Monitor::v_readwrite( Msg * msg)
{
   rw_int32_t( m_id,            msg);
   rw_int32_t( m_uid,           msg);
   rw_int64_t( m_time_launch,   msg);
   rw_int64_t( m_time_register, msg);
   rw_int64_t( m_time_activity, msg);
   rw_String ( m_name,          msg);
   rw_String ( m_user_name,     msg);
	rw_String ( m_engine,        msg);

	for( int e = 0; e < m_events.size(); e++)
	{
		bool b = m_events[e];
		rw_bool( b, msg);
		m_events[e] = b;
	}

   rw_Int32_List( m_jobsIds,      msg);

   m_address.v_readwrite( msg);
}

void Monitor::v_jsonWrite( std::ostringstream & o_str, int i_type) const
{
	o_str << "{";

	Client::v_jsonWrite( o_str, i_type);

	o_str << "\n,\"uid\":" << m_uid;
	o_str << "\n,\"time_activity\":" << m_time_activity;

	if( m_events.size())
	{
		o_str << "\n,\"events\":[";
		bool evt_written = false;
		for( int e = 0; e < m_events.size(); e++)
		{
			if( m_events[e] != true )
				continue;

			if( evt_written ) o_str << ',';

			o_str << "\"" << EVT_NAMES[e] << "\"";
			evt_written = true;
		}
		o_str << "]";
	}


	if( m_jobsIds.size())
		jw_int32list("jobs_ids", m_jobsIds, o_str);

	o_str << "\n}";
}

bool Monitor::hasEvent( int i_type) const
{
	if( i_type < 0 )
	{
		AFERRAR("Monitor::hasEvent: Event type is negative: %d", i_type)
		return false;
	}
	if( i_type >= EVT_COUNT )
	{
		AFERRAR("Monitor::hasEvent: Event type is too big: %d", i_type)
		return false;
	}

	return m_events[i_type];
}

void Monitor::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( full )
   {
      stream << "Monitor name = \"" << m_name << "\" (id=" << getId() << ")";
		stream << "\n Engine: " << m_engine;
      stream << "\n Launch Time: " + af::time2str( m_time_launch);
      stream << "\n Register Time: " + af::time2str( m_time_register);
		if( m_time_activity )
			stream << "\n Time Activity: " + af::time2str( m_time_activity);

      stream << "\n JIds[" << m_jobsIds.size() << "]:";
      for( std::list<int32_t>::const_iterator it = m_jobsIds.begin(); it != m_jobsIds.end(); it++)
         stream << " " << *it;

		stream << "\n System Events:";
		for( int e = 0; e < EVT_COUNT; e++)
		{
			stream << "\n    " << EVT_NAMES[e] << " : ";
			if( hasEvent(e)) stream << " SUBMITTED";
			else stream << "   ---";
		}
   }
   else
   {
      stream << m_name << "[" << m_id << "]";
      stream << " e'" << m_engine << "' ";
      m_address.v_generateInfoStream( stream, full);
   }
}

const char * Monitor::EVT_NAMES[] =
{
	"jobs_add",
	"jobs_change",
	"jobs_del",
	"JOBS_COUNT",
	"users_add",
	"users_change",
	"users_del",
	"renders_add",
	"renders_change",
	"renders_del",
	"monitors_add",
	"monitors_change",
	"monitors_del",
	"COUNT"
};

