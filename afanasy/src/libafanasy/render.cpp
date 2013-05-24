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
   Client( Client::GetEnvironment, 0)
{
	m_state = State;
	construct();
	m_priority = Priority;
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
	m_max_tasks = -1;
	m_capacity = -1;
	m_capacity_used = 0;
	m_wol_operation_time = 0;
	m_idle_time = 0;
	m_busy_time = 0;
}

Render::~Render()
{
}


void Render::v_jsonWrite( std::ostringstream & o_str, int i_type) const
{
	o_str << "{";

	// Write running tasks percents in any case:
	if( m_tasks.size())
	{
		o_str << "\"tasks_percents\":[";
		bool first = true;
		for( std::list<TaskExec*>::const_iterator it = m_tasks.begin(); it != m_tasks.end(); it++)
		{
			if( false == first )
				o_str << ',';
			else
				first = false;

			o_str << (*it)->getPercent();
		}
		o_str << "],";
	}

	if( i_type == af::Msg::TRendersResources )
	{
		o_str << "\"id\":"   << m_id;
		if( isOnline())
		{
			o_str << ",\"idle_time\":" << m_idle_time;
			o_str << ",\"busy_time\":" << m_busy_time;
			o_str << ",";
			m_hres.jsonWrite( o_str);
		}
		o_str << "}";
		return;
	}

	Client::v_jsonWrite( o_str, i_type);

	o_str << ",";
	jw_state( m_state, o_str, true /*it is render node state type*/);

	o_str << ",\"capacity_used\":" << m_capacity_used;
	o_str << ",\"task_start_finish_time\":" << m_task_start_finish_time;
	if( m_capacity  > 0 )
		o_str << ",\"capacity\":" << m_capacity;
	if( m_max_tasks  > 0 )
		o_str << ",\"max_tasks\":" << m_max_tasks;
	if( m_wol_operation_time > 0 )
		o_str << ",\"wol_operation_time\":" << m_wol_operation_time;
	o_str << ",\"idle_time\":" << m_idle_time;
	o_str << ",\"busy_time\":" << m_busy_time;

	if( m_tasks.size())
	{
		o_str << ",\"tasks\":[";
		bool first = true;
		for( std::list<TaskExec*>::const_iterator it = m_tasks.begin(); it != m_tasks.end(); it++)
		{
			if( false == first )
				o_str << ',';
			else
				first = false;

			(*it)->jsonWrite( o_str, i_type);
		}
		o_str << "]";
	}

	o_str << ',';
	m_host.jsonWrite( o_str);

	o_str << "}";
}

void Render::jsonRead( const JSON &i_object, std::string * io_changes)
{
	if( false == i_object.IsObject())
	{
		AFERROR("Render::jsonRead: Not a JSON object.")
		return;
	}

	jr_string("user_name",   m_user_name,   i_object, io_changes);
	jr_int32 ("capacity",    m_capacity,    i_object, io_changes);
	jr_int32 ("max_tasks",   m_max_tasks,   i_object, io_changes);
	checkDirty();

	bool nimby, NIMBY;
	if( jr_bool("nimby", nimby, i_object, io_changes))
	{
		if( nimby ) setNimby();
		else setFree();
	}
	if( jr_bool("NIMBY", NIMBY, i_object, io_changes))
	{
		if( NIMBY ) setNIMBY();
		else setFree();
	}
}

void Render::v_readwrite( Msg * msg)
{
   switch( msg->type())
   {
   case Msg::TRendersList:

	  rw_bool   ( m_locked,                 msg);
	  rw_int64_t( m_task_start_finish_time, msg);
	  rw_int32_t( m_max_tasks,              msg);
	  rw_int32_t( m_capacity,               msg);
	  rw_int32_t( m_capacity_used,          msg);
	  rw_int64_t( m_time_update,            msg);
	  rw_int64_t( m_time_register,          msg);
	  rw_int64_t( m_wol_operation_time,     msg);
	  rw_String ( m_annotation,             msg);

      if( msg->isWriting())
      {
		 uint32_t taskscount = uint32_t(m_tasks.size());
         rw_uint32_t( taskscount, msg);
		 std::list<TaskExec*>::iterator it = m_tasks.begin();
         for( unsigned t = 0; t < taskscount; t++) (*(it++))->write( msg);
      }
      else
      {
         uint32_t taskscount;
         rw_uint32_t( taskscount, msg);
		 for( unsigned t = 0; t < taskscount; t++) m_tasks.push_back( new TaskExec( msg));
      }

   case Msg::TRenderRegister:

	  rw_String  ( m_version,      msg);
	  rw_String  ( m_name,         msg);
	  rw_String  ( m_user_name,    msg);
	  rw_uint32_t( m_state,        msg);
	  rw_uint32_t( m_flags,        msg);
	  rw_uint8_t ( m_priority,     msg);
	  rw_int64_t ( m_time_launch,  msg);
	  m_host.v_readwrite( msg);
	  m_address.v_readwrite( msg);

   case Msg::TRenderUpdate:
   case Msg::TRendersResources:

		// Tasks percents, needed for GUI only:
		if( msg->isWriting())
		{
			m_tasks_percents.clear();
			for( std::list<TaskExec*>::iterator it = m_tasks.begin(); it != m_tasks.end(); it++)
				m_tasks_percents.push_back((*it)->getPercent());
		}
		rw_Int32_Vect( m_tasks_percents, msg);

		rw_int64_t( m_idle_time, msg);
		rw_int64_t( m_busy_time, msg);
	  m_hres.v_readwrite( msg);

      break;
   default:
      AFERROR("Render::readwrite(): invalid type.\n");
   }

   // Always send ID
   rw_int32_t( m_id, msg);

   // Send network interfaces information only when register
   if( msg->type() == Msg::TRenderRegister)
   {
	  int8_t netIfs_size = m_netIFs.size();
      rw_int8_t( netIfs_size, msg);
      for( int i = 0; i < netIfs_size; i++)
		 if( msg->isWriting()) m_netIFs[i]->write( msg);
		 else m_netIFs.push_back( new NetIF( msg));
   }
}

void Render::checkDirty()
{
   if( m_capacity == m_host.m_capacity ) m_capacity = -1;
   if( m_max_tasks == m_host.m_max_tasks ) m_max_tasks = -1;
   if(( m_capacity == -1 ) && ( m_max_tasks == -1 ) && ( m_services_disabled.empty() ))
	  m_state = m_state & (~SDirty);
   else
	  m_state = m_state | SDirty;
}

int Render::v_calcWeight() const
{
   int weight = Client::v_calcWeight();
   weight += sizeof(Render) - sizeof( Client);
   for( std::list<TaskExec*>::const_iterator it = m_tasks.begin(); it != m_tasks.end(); it++) weight += (*it)->calcWeight();
   return weight;
}

void Render::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( full)
   {
	  stream << "Render " << m_name << "@" << m_user_name << " (id=" << m_id << "):";
	  stream << "\n Version = \"" << m_version;

      if( isDirty()) stream << "\nDirty! Capacity|Max Tasks changed, or service(s) disabled.";

      stream << std::endl;
	  m_address.v_generateInfoStream( stream ,full);

		stream << "\n Status:";
		if( isOnline()) stream << " Online";
		if( isOffline()) stream << " Offline";
		if( isHidden()) stream << " Hidden";
		if( isBusy()) stream << " Busy";
		if( isNimby()) stream << " (nimby)";
		if( isNIMBY()) stream << " (NIMBY)";
		if( isWOLFalling()) stream << " WOL-Falling";
		if( isWOLSleeping()) stream << " WOL-Sleeping";
		if( isWOLWaking()) stream << " WOL-Waking";

	  stream << "\n Priority = " << int(m_priority);
      stream << "\n Capacity = " << getCapacityFree() << " of " << getCapacity() << " ( " << getCapacityUsed() << " used )";
      stream << "\n Max Tasks = " << getMaxTasks() << " ( " << getTasksNumber() << " running )";

	  if( m_wol_operation_time ) stream << "\n WOL operation time = " << time2str( m_wol_operation_time);
		stream << "\n Idle Time = " << time2str( m_idle_time) << " CPU < " << m_host.m_idle_cpu << "%";
		stream << "\n Busy Time = " << time2str( m_busy_time) << " CPU > " << m_host.m_busy_cpu << "%";
	  if( m_time_launch   ) stream << "\n Launched at: " << time2str( m_time_launch   );
	  if( m_time_register ) stream << "\n Registered at: " << time2str( m_time_register );

      stream << std::endl;
	  m_host.v_generateInfoStream( stream, full);

	  if( m_netIFs.size())
      {
         stream << "\nNetwork Interfaces:";
		 for( int i = 0; i < m_netIFs.size(); i++)
         {
            stream << "\n   ";
			m_netIFs[i]->v_generateInfoStream( stream, true);
         }
      }

//      hres.generateInfoStream( stream ,full);
   }
   else
   {
		if( isOnline())     stream << " ON ";
		if( isOffline())    stream << " off";

		if( isBusy()) stream << " BUSY"; else stream << "     ";

		if( isWOLFalling())  stream << " WFL"; else stream << "    ";
		if( isWOLSleeping()) stream << " WSL"; else stream << "    ";
		if( isWOLWaking())   stream << " WWK"; else stream << "    ";

		if( isNimby())      stream << " n";
		else if( isNIMBY()) stream << " N";
		else                stream << "  ";

		stream << " " << m_name << "@" << m_user_name << "[" << m_id << "]";

		if( m_wol_operation_time ) stream << " W:" << time2str( m_wol_operation_time);
		stream << " I:" << time2str( m_idle_time);
		stream << " B:" << time2str( m_busy_time);

		stream << " v'" << m_version << "'";
		stream << " ";
		m_address.v_generateInfoStream( stream ,full);
//      stream << " - " << calcWeight() << " bytes.";
   }
}
