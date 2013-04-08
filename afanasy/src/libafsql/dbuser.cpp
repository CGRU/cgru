#include "dbuser.h"

#include "dbattr.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afsql;

const std::string DBUser::TableName("users");
//const std::string DBUser::Keys("password varchar(128), administrator boolean DEFAULT FALSE, PRIMARY KEY( id)");
const std::string DBUser::Keys("PRIMARY KEY( id)");
const int         DBUser::KeysNum = 1;

DBUser::DBUser( const std::string & username, const std::string & host):
   af::User( username, host)
{
   addDBAttributes();
}

DBUser::DBUser( int uid):
   af::User( uid)
{
//printf("DBUser::DBUser: id=%d\n", uid);
   addDBAttributes();
}

void DBUser::addDBAttributes()
{
   dbAddAttr( new DBAttrInt32(  DBAttr::_id,                   &m_id                  ));

   dbAddAttr( new DBAttrUInt32( DBAttr::_state,                &m_state               ));
   dbAddAttr( new DBAttrUInt32( DBAttr::_flags,                &m_flags               ));
   dbAddAttr( new DBAttrString( DBAttr::_hostname,             &m_host_name            ));
   dbAddAttr( new DBAttrInt32 ( DBAttr::_maxrunningtasks,      &m_max_running_tasks     ));
   dbAddAttr( new DBAttrUInt8 ( DBAttr::_priority,             &m_priority            ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_hostsmask,            &m_hosts_mask           ));
   dbAddAttr( new DBAttrUInt8 ( DBAttr::_errors_retries,       &m_errors_retries      ));
   dbAddAttr( new DBAttrUInt8 ( DBAttr::_errors_avoidhost,     &m_errors_avoid_host    ));
   dbAddAttr( new DBAttrUInt8 ( DBAttr::_errors_tasksamehost,  &m_errors_task_same_host ));
   dbAddAttr( new DBAttrInt32 ( DBAttr::_errors_forgivetime,   &m_errors_forgive_time  ));
   dbAddAttr( new DBAttrInt32 ( DBAttr::_lifetime,             &m_jobs_life_time       ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_hostsmask_exclude,    &m_hosts_mask_exclude   ));
   dbAddAttr( new DBAttrInt64 ( DBAttr::_time_register,        &m_time_register       ));
   dbAddAttr( new DBAttrString( DBAttr::_annotation,           &m_annotation          ));
   dbAddAttr( new DBAttrString( DBAttr::_customdata,           &m_custom_data          ));

   dbAddAttr( new DBAttrString( DBAttr::_name,                 &m_name                ));

// Unused by server, but used in vebvisor:
   password = "";
   dbAddAttr( new DBAttrString( DBAttr::_password,             &password            ));
}

DBUser::~DBUser()
{
}

const std::string DBUser::dbGetIDsCmd()
{
   return std::string("SELECT id FROM ") + TableName;
}

bool DBUser::v_dbSelect( PGconn * i_conn, const std::string * i_where)
{
//printf("DBUser::dbSelect:\n");
   if( DBItem::v_dbSelect( i_conn, i_where) == false) return false;

// This user came from database on core init, so he is permanent
   setPermanent( true);

   return true;
}
