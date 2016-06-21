#include "render.h"

#include "address.h"
#include "environment.h"
#include "msg.h"
#include "taskexec.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "logger.h"

using namespace af;

Render::Render(  Client::Flags i_flags):
   Client( i_flags, 0)
{
	construct();

	if( i_flags != Client::DoNotGetAnyValues )
	{
		m_state = af::Render::SOnline;
		if( af::Environment::hasArgument("-NIMBY"))
		{
			printf("Initial state set to 'NIMBY'\n");
			m_state = m_state | af::Render::SNIMBY;
		}
		else if( af::Environment::hasArgument("-nimby"))
		{
			printf("Initial state set to 'nimby'\n");
			m_state = m_state | af::Render::Snimby;
		}
		m_priority = af::Environment::getPriority();
		m_capacity = af::Environment::getRenderDefaultCapacity();
	}
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
	std::list<af::TaskExec*>::iterator it;
	for( it = m_tasks.begin(); it != m_tasks.end(); it++)
		delete *it;
}

void Render::v_jsonWrite( std::ostringstream & o_str, int i_type) const // Thread-safe
{
	o_str << "{";

	// Write running tasks percents in any case:
	if( m_tasks.size())
	{
		o_str << "\n\"tasks_percents\":[";
		bool first = true;
		for( std::list<TaskExec*>::const_iterator it = m_tasks.begin(); it != m_tasks.end(); it++)
		{
			if( false == first )
				o_str << ",\n";
			else
				first = false;

			o_str << (*it)->getPercent();
		}
		o_str << "\n],";
	}

	if( i_type == af::Msg::TRendersResources )
	{
		o_str << "\n\"id\":"   << m_id;
		if( isOnline())
		{
			o_str << ",\n\"idle_time\":" << m_idle_time;
			o_str << ",\n\"busy_time\":" << m_busy_time;
			o_str << ",\n";
			m_hres.jsonWrite( o_str);
		}
		o_str << "\n}";
		return;
	}

	Client::v_jsonWrite( o_str, i_type);

	o_str << ",\n\"st\":" << m_state;
	o_str << ",\n";
	jw_state( m_state, o_str, true /*it is render node state type*/);

	o_str << ",\n\"capacity_used\":" << m_capacity_used;
	o_str << ",\n\"task_start_finish_time\":" << m_task_start_finish_time;
	if( m_capacity  > 0 )
		o_str << ",\n\"capacity\":" << m_capacity;
	if( m_max_tasks  > 0 )
		o_str << ",\n\"max_tasks\":" << m_max_tasks;
	if( m_wol_operation_time > 0 )
		o_str << ",\n\"wol_operation_time\":" << m_wol_operation_time;
	o_str << ",\n\"idle_time\":" << m_idle_time;
	o_str << ",\n\"busy_time\":" << m_busy_time;

	// Write tasks if any.
	// We do not need to store tasks on hdd (when type is zero).
	if( m_tasks.size() && ( i_type != 0 ))
	{
		o_str << ",\n\"tasks\":[";
		bool first = true;
		for( std::list<TaskExec*>::const_iterator it = m_tasks.begin(); it != m_tasks.end(); it++)
		{
			if( false == first )
				o_str << ",\n";
			else
				first = false;

			(*it)->jsonWrite( o_str, i_type);
		}
		o_str << "\n]";
	}

	// We do not need to store host on hdd,
	// it will be taken from farm setup when online.
	if( i_type != 0 )
	{
		o_str << ",\n";
		m_host.jsonWrite( o_str);
	}

	if( m_services_disabled.size())
	{
		o_str << ",\n\"services_disabled\":[";
		for( int i = 0; i < m_services_disabled.size(); i++)
		{
			if( i ) o_str << ",";
			o_str << '\"' << m_services_disabled[i] << '\"';
		}
		o_str << ']';
	}

	o_str << "\n}";
}

bool Render::jsonRead( const JSON &i_object, std::string * io_changes)
{
	if( false == i_object.IsObject())
	{
		AFERROR("Render::jsonRead: Not a JSON object.")
		return false;
	}

	jr_string("user_name",   m_user_name,   i_object, io_changes);
	jr_int32 ("capacity",    m_capacity,    i_object, io_changes);
	jr_int32 ("max_tasks",   m_max_tasks,   i_object, io_changes);
	checkDirty();

	bool nimby, NIMBY, paused;
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
	if( jr_bool("paused", paused, i_object, io_changes))
	{
		setPaused( paused);
	}

	// Paramers below are not editable and read only on creation
	// ( but they can be changes by other actions, like disable service)
	// When use edit parameters, log provided to store changes
	if( io_changes )
		return true;

	Node::jsonRead( i_object);

	jr_int64("st", m_state, i_object);

	Client::jsonRead( i_object);

	jr_stringvec("services_disabled", m_services_disabled, i_object);

	return true;
}

void Render::v_readwrite( Msg * msg) // Thread-safe
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

	case Msg::TRenderRegister:
 
		// Writing tasks execs, needed for:
		// - GUIs to show tasks in render item.
		// - Server for running tasks reconnection at startup.
		{
			uint32_t taskscount = uint32_t(m_tasks.size());
			rw_uint32_t( taskscount, msg);

			if( msg->isWriting())
			{
				for( std::list<TaskExec*>::iterator it = m_tasks.begin(); it != m_tasks.end(); it++)
					(*it)->write( msg);
			}
			else
			{
				for( unsigned t = 0; t < taskscount; t++)
					m_tasks.push_back( new TaskExec( msg));
			}
		}

	  rw_String  ( m_engine,       msg);
	  rw_String  ( m_name,         msg);
	  rw_String  ( m_user_name,    msg);
	  rw_int64_t ( m_state,        msg);
	  rw_int64_t ( m_flags,        msg);
	  rw_uint8_t ( m_priority,     msg);
	  rw_int64_t ( m_time_launch,  msg);
	  m_host.v_readwrite( msg);
	  m_address.v_readwrite( msg);

   case Msg::TRenderUpdate:
   case Msg::TRendersResources:

		// Tasks percents, needed for GUI only:
		if( msg->isWriting())
		{
			// Fill in temporary array,
			// not m_tasks_percents (which is used on client side for reading only)
			// because this function should be thread-safe (we can't clear() m_tasks_percents here)
			std::vector<int32_t> _tasks_percents;
			for( std::list<TaskExec*>::iterator it = m_tasks.begin(); it != m_tasks.end(); it++)
				_tasks_percents.push_back((*it)->getPercent());
			rw_Int32_Vect( _tasks_percents, msg);
		}
		else
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

std::list<TaskExec *> Render::takeTasks()
{
	std::list<af::TaskExec*> l = m_tasks;
	m_tasks.clear();
	return l;
}

void Render::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( full)
   {
	  stream << "Render " << m_name << "@" << m_user_name << " (id=" << m_id << "):";
	  stream << "\n Engine = \"" << m_engine;

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
		if( isPaused()) stream << " (Paused)";
		if( isWOLFalling()) stream << " WOL-Falling";
		if( isWOLSleeping()) stream << " WOL-Sleeping";
		if( isWOLWaking()) stream << " WOL-Waking";

		stream << "\n Priority = " << int(m_priority);
		stream << "\n Capacity = " << getCapacityFree() << " of " << getCapacity() << " ( " << getCapacityUsed() << " used )";
		stream << "\n Max Tasks = " << getMaxTasks() << " ( " << getTasksNumber() << " running )";

		if( m_wol_operation_time ) stream << "\n WOL operation time = " << time2str( m_wol_operation_time);

		stream << "\n Idle Time = " << time2str( m_idle_time);
		stream << "\n Busy Time = " << time2str( m_busy_time);

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

		m_hres.v_generateInfoStream( stream ,full);
   }
   else
   {
		if( isOnline())     stream << " ON ";
		if( isOffline())    stream << " off";

		if( isBusy()) stream << " BUSY"; else stream << "     ";

		if( isWOLFalling())  stream << " WFL"; else stream << "    ";
		if( isWOLSleeping()) stream << " WSL"; else stream << "    ";
		if( isWOLWaking())   stream << " WWK"; else stream << "    ";

		if( isNimby())           stream << " n";
		else if( isNIMBY())      stream << " N";
		else if( isPaused()) stream << " P";
		else                     stream << "  ";

		stream << " " << m_name << "@" << m_user_name << "[" << m_id << "]";
/*
		if( m_wol_operation_time ) stream << " W:" << time2str( m_wol_operation_time);
		stream << " I:" << time2str( m_idle_time);
		stream << " B:" << time2str( m_busy_time);

		stream << " e'" << m_engine << "'";
*/
		stream << " ";
		m_address.v_generateInfoStream( stream ,full);
   }
}
