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
}

Render::~Render()
{
}

void Render::readwrite( Msg * msg)
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
	  m_host.readwrite( msg);
	  m_address.readwrite( msg);

   case Msg::TRenderUpdate:
   case Msg::TRendersListUpdates:

	  m_hres.readwrite( msg);

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
   if( m_capacity == m_host.capacity ) m_capacity = -1;
   if( m_max_tasks == m_host.maxtasks ) m_max_tasks = -1;
   if(( m_capacity == -1 ) && ( m_max_tasks == -1 ) && ( m_services_disabled.empty() ))
	  m_state = m_state | SDirty;
   else
	  m_state = m_state & (~SDirty);
}

void Render::restoreDefaults()
{
   m_capacity = -1;//host.capacity;
   m_max_tasks = -1;//host.maxtasks;
   m_services_disabled.clear();
   m_state = m_state & (~SDirty);
}

int Render::calcWeight() const
{
   int weight = Client::calcWeight();
   weight += sizeof(Render) - sizeof( Client);
   for( std::list<TaskExec*>::const_iterator it = m_tasks.begin(); it != m_tasks.end(); it++) weight += (*it)->calcWeight();
   return weight;
}

void Render::generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( full)
   {
	  stream << "Render " << m_name << "@" << m_user_name << " (id=" << m_id << "):";
	  stream << "\n Version = \"" << m_version;

      if( isDirty()) stream << "\nDirty! Capacity|Max Tasks changed, or service(s) disabled.";

      stream << std::endl;
	  m_address.generateInfoStream( stream ,full);

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
	  if( m_time_launch   ) stream << "\n Launched at: " << time2str( m_time_launch   );
	  if( m_time_register ) stream << "\n Registered at: " << time2str( m_time_register );

      stream << std::endl;
	  m_host.generateInfoStream( stream, full);

	  if( m_netIFs.size())
      {
         stream << "\nNetwork Interfaces:";
		 for( int i = 0; i < m_netIFs.size(); i++)
         {
            stream << "\n   ";
			m_netIFs[i]->generateInfoStream( stream, true);
         }
      }

//      hres.generateInfoStream( stream ,full);
   }
   else
   {
		if( isOnline())  stream << " ON  ";
		if( isOffline()) stream << " off ";
		stream << m_name << "@" << m_user_name << "[" << m_id << "]";
		stream << " v'" << m_version << "'";
		if( isBusy())  stream << " B";
		if( isNimby()) stream << " (n)";
		if( isNIMBY()) stream << " (N)";
		stream << " ";
		m_address.generateInfoStream( stream ,full);
//      stream << " - " << calcWeight() << " bytes.";
   }
}
