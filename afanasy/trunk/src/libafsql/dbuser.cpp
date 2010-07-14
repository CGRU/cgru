#include "dbuser.h"

#include "dbattr.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afsql;

const QString DBUser::TableName("users");
const QString DBUser::Keys("password varchar(128), administrator boolean DEFAULT FALSE, PRIMARY KEY( id)");
const int     DBUser::KeysNum = 1;

DBUser::DBUser( const QString &username, const QString &host):
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
   dbAddAttr( new DBAttrInt32(  DBAttr::_maxhosts,             &maxhosts            ));
   dbAddAttr( new DBAttrUInt8(  DBAttr::_priority,             &priority            ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_hostsmask,            &hostsmask           ));
   dbAddAttr( new DBAttrUInt8(  DBAttr::_errors_retries,       &errors_retries      ));
   dbAddAttr( new DBAttrUInt8(  DBAttr::_errors_avoidhost,     &errors_avoidhost    ));
   dbAddAttr( new DBAttrUInt8(  DBAttr::_errors_tasksamehost,  &errors_tasksamehost ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_hostsmask_exclude,    &hostsmask_exclude   ));
   dbAddAttr( new DBAttrUInt32( DBAttr::_time_register,        &time_register       ));
   dbAddAttr( new DBAttrString( DBAttr::_customdata,           &customdata          ));

   dbAddAttr( new DBAttrString( DBAttr::_name,                 &name                ));
}

DBUser::~DBUser()
{
}

void DBUser::getIds( std::list<int32_t> & uids, QSqlDatabase * db)
{
   if( db->isOpen() == false )
   {
      AFERROR("DBUser::getIds: Database connection is not open\n");
      return;
   }
   QSqlQuery q( *db);
   q.exec(QString("SELECT id FROM %1").arg(TableName));
   while (q.next()) uids.push_back(q.value(0).toUInt());
   printf("DBUser::getIds: %u users founded.\n", unsigned(uids.size()));
}

bool DBUser::dbSelect( QSqlDatabase * db, const QString * where)
{
//printf("DBUser::dbSelect:\n");
   if( DBItem::dbSelect( db, where) == false) return false;

// This user came from database on core init, so he is permanent
   setPermanent( true);

   return true;
}
