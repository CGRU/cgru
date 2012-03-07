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
   dbAddAttr( new DBAttrInt32(  DBAttr::_id,                   &id                  ));

   dbAddAttr( new DBAttrUInt32( DBAttr::_state,                &state               ));
   dbAddAttr( new DBAttrUInt32( DBAttr::_flags,                &flags               ));
   dbAddAttr( new DBAttrString( DBAttr::_hostname,             &hostname            ));
   dbAddAttr( new DBAttrInt32 ( DBAttr::_maxrunningtasks,      &maxrunningtasks     ));
   dbAddAttr( new DBAttrUInt8 ( DBAttr::_priority,             &priority            ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_hostsmask,            &hostsmask           ));
   dbAddAttr( new DBAttrUInt8 ( DBAttr::_errors_retries,       &errors_retries      ));
   dbAddAttr( new DBAttrUInt8 ( DBAttr::_errors_avoidhost,     &errors_avoidhost    ));
   dbAddAttr( new DBAttrUInt8 ( DBAttr::_errors_tasksamehost,  &errors_tasksamehost ));
   dbAddAttr( new DBAttrInt32 ( DBAttr::_errors_forgivetime,   &errors_forgivetime  ));
   dbAddAttr( new DBAttrInt32 ( DBAttr::_lifetime,             &jobs_lifetime       ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_hostsmask_exclude,    &hostsmask_exclude   ));
   dbAddAttr( new DBAttrInt64 ( DBAttr::_time_register,        &time_register       ));
   dbAddAttr( new DBAttrString( DBAttr::_annotation,           &annotation          ));
   dbAddAttr( new DBAttrString( DBAttr::_customdata,           &customdata          ));

   dbAddAttr( new DBAttrString( DBAttr::_name,                 &name                ));

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

bool DBUser::dbSelect( PGconn * i_conn, const std::string * i_where)
{
//printf("DBUser::dbSelect:\n");
   if( DBItem::dbSelect( i_conn, i_where) == false) return false;

// This user came from database on core init, so he is permanent
   setPermanent( true);

   return true;
}
