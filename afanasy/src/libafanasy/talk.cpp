#include "talk.h"

#include <stdio.h>

#include "environment.h"
#include "address.h"
#include "msg.h"

using namespace af;

Talk::Talk():
   Client( Client::GetEnvironment, 0)
{
   m_name = af::Environment::getUserName() + "@" + af::Environment::getHostName() + ":" + m_address.generatePortString();
}

Talk::Talk( Msg * msg):
   Client( Client::DoNotGetAnyValues, 0)
{
   read( msg);
}

Talk::~Talk()
{
}

void Talk::v_readwrite( Msg * msg)
{
   rw_int32_t( m_id,            msg);
   rw_int64_t( m_time_launch,   msg);
   rw_int64_t( m_time_update,   msg);
   rw_int64_t( m_time_register, msg);
   rw_String ( m_name,          msg);
   rw_String ( m_user_name,      msg);
   rw_String ( m_version,       msg);
   m_address.v_readwrite( msg);
}

void Talk::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( full )
   {
      stream << m_name << " (id=" << getId() << "):";
      stream << "\n Launched At: " << af::time2str( m_time_launch);
      stream << "\n Registered At: " << af::time2str( m_time_register);
      stream << "\n Version: " << m_version;
      stream << "\n Last Update At: " << af::time2str( m_time_update);
   }
   else
   {
      stream << m_name << "[" << m_id << "]";
      stream << " v'" << m_version << "' ";
      m_address.v_generateInfoStream( stream, full);
   }
}
