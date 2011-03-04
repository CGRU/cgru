#include "dbrender.h"

#include "dbattr.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afsql;

const QString DBRender::TableName("renders");
const QString DBRender::Keys("PRIMARY KEY( id)");
const int     DBRender::KeysNum = 1;

DBRender::DBRender( int Id):
   af::Render( Id)
{
   addDBAttributes();
}

DBRender::DBRender( af::Msg * msg, const af::Address * addr):
   af::Render( msg, addr)
{
   addDBAttributes();
}

void DBRender::addDBAttributes()
{
   dbAddAttr( new DBAttrInt32(  DBAttr::_id,                &id                  ));

   dbAddAttr( new DBAttrUInt32(  DBAttr::_state,             &state               ));
   dbAddAttr( new DBAttrUInt8(   DBAttr::_priority,          &priority            ));
   dbAddAttr( new DBAttrString(  DBAttr::_username,          &username            ));
   dbAddAttr( new DBAttrInt32(   DBAttr::_capacity,          &capacity            ));
   dbAddAttr( new DBAttrString(  DBAttr::_services_disabled, &services_disabled   ));
   dbAddAttr( new DBAttrString(  DBAttr::_annotation,        &annotation          ));
   dbAddAttr( new DBAttrString(  DBAttr::_customdata,        &customdata          ));

   dbAddAttr( new DBAttrString(  DBAttr::_name,               &name               ));
}

DBRender::~DBRender()
{
}

void DBRender::getIds( std::list<int32_t> & uids, QSqlDatabase * db)
{
   if( db->isOpen() == false )
   {
      AFERROR("DBRender::getIds: Database connection is not open\n");
      return;
   }
   QSqlQuery q( *db);
   q.exec(QString("SELECT id FROM %1").arg(TableName));
   while (q.next()) uids.push_back(q.value(0).toUInt());
   printf("DBRender::getIds: %u renders founded.\n", unsigned(uids.size()));
}

bool DBRender::dbSelect( QSqlDatabase * db, const QString * where)
{
//printf("DBRender::dbSelect:\n");
   if( DBItem::dbSelect( db, where) == false) return false;

// This render came from database on core init, it can't be online or busy
   setOffline();
   setBusy( false);

   return true;
}
