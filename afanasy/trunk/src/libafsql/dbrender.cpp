#include "dbrender.h"

#include "dbattr.h"

#include <QtCore/QVariant>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afsql;

const std::string DBRender::TableName("renders");
const std::string DBRender::Keys("PRIMARY KEY( id)");
const int         DBRender::KeysNum = 1;

DBRender::DBRender( int Id):
   af::Render( Id)
{
   addDBAttributes();
}

DBRender::DBRender( af::Msg * msg):
   af::Render( msg)
{
   addDBAttributes();
}

void DBRender::addDBAttributes()
{
   dbAddAttr( new DBAttrInt32 (  DBAttr::_id,                &id                 ));

   dbAddAttr( new DBAttrUInt32(  DBAttr::_state,             &state              ));
   dbAddAttr( new DBAttrUInt8 (  DBAttr::_priority,          &priority           ));
   dbAddAttr( new DBAttrString(  DBAttr::_username,          &username           ));
   dbAddAttr( new DBAttrInt32 (  DBAttr::_capacity,          &capacity           ));
   dbAddAttr( new DBAttrInt32 (  DBAttr::_maxrunningtasks,   &maxtasks           ));
   dbAddAttr( new DBAttrString(  DBAttr::_services_disabled, &services_disabled  ));
   dbAddAttr( new DBAttrString(  DBAttr::_annotation,        &annotation         ));
   dbAddAttr( new DBAttrString(  DBAttr::_customdata,        &customdata         ));
   dbAddAttr( new DBAttrString(  DBAttr::_ipaddresses,       &ipaddresses        ));
   dbAddAttr( new DBAttrString(  DBAttr::_macaddresses,      &macaddresses       ));

   dbAddAttr( new DBAttrString(  DBAttr::_name,              &name               ));
}

DBRender::~DBRender()
{
}

const std::string DBRender::dbGetIDsCmd()
{
   return std::string("SELECT id FROM ") + TableName;
}

void DBRender::dbInsert( std::list<std::string> * queries) const
{
   updateNetIFs();
   DBItem::dbInsert( queries);
}
void DBRender::dbUpdate( std::list<std::string> * queries, int attr) const
{
   updateNetIFs();
   DBItem::dbUpdate( queries, attr);
}
bool DBRender::dbSelect( QSqlDatabase * db, const std::string * where)
{
//printf("DBRender::dbSelect:\n");
   if( DBItem::dbSelect( db, where) == false) return false;

// This render came from database on core init, it can't be online or busy
   setOffline();
   setBusy( false);

   af::NetIF::getNetIFs( macaddresses, netIFs);
   address = af::Address( ipaddresses);

   return true;
}

void DBRender::updateNetIFs() const
{
   macaddresses.clear();
   for( int i = 0; i < netIFs.size(); i++)
   {
      if( i != 0 ) macaddresses += " ";
      macaddresses += netIFs[i]->getMACAddrString( true /* with separators */);
   }
   ipaddresses.clear();
   ipaddresses = address.generateIPString( false);
}
