#include "dbrender.h"

#include "dbattr.h"

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
   dbAddAttr( new DBAttrInt32 (  DBAttr::_id,                &m_id                 ));

   dbAddAttr( new DBAttrUInt32(  DBAttr::_state,             &m_state              ));
   dbAddAttr( new DBAttrUInt32(  DBAttr::_flags,             &m_flags              ));
   dbAddAttr( new DBAttrUInt8 (  DBAttr::_priority,          &m_priority           ));
   dbAddAttr( new DBAttrString(  DBAttr::_username,          &m_user_name           ));
   dbAddAttr( new DBAttrInt32 (  DBAttr::_capacity,          &m_capacity           ));
   dbAddAttr( new DBAttrInt32 (  DBAttr::_maxrunningtasks,   &m_max_tasks           ));
   dbAddAttr( new DBAttrString(  DBAttr::_services_disabled, &m_services_disabled  ));
   dbAddAttr( new DBAttrString(  DBAttr::_annotation,        &m_annotation         ));
   dbAddAttr( new DBAttrString(  DBAttr::_customdata,        &m_custom_data         ));
   dbAddAttr( new DBAttrString(  DBAttr::_ipaddresses,       &ipaddresses        ));
   dbAddAttr( new DBAttrString(  DBAttr::_macaddresses,      &macaddresses       ));

   dbAddAttr( new DBAttrString(  DBAttr::_name,              &m_name               ));
}

DBRender::~DBRender()
{
}

const std::string DBRender::dbGetIDsCmd()
{
   return std::string("SELECT id FROM ") + TableName;
}

void DBRender::v_dbInsert( std::list<std::string> * queries) const
{
   updateNetIFs();
   DBItem::v_dbInsert( queries);
}
void DBRender::v_dbUpdate( std::list<std::string> * queries, int attr) const
{
   updateNetIFs();
   DBItem::v_dbUpdate( queries, attr);
}
bool DBRender::v_dbSelect( PGconn * i_conn, const std::string * i_where)
{
//printf("DBRender::dbSelect:\n");
   if( DBItem::v_dbSelect( i_conn, i_where) == false) return false;

// This render came from database on core init, it can't be online or busy
   setOffline();
   setBusy( false);

   af::NetIF::getNetIFs( macaddresses, m_netIFs);
   m_address = af::Address( ipaddresses);

   return true;
}

void DBRender::updateNetIFs() const
{
   macaddresses.clear();
   for( int i = 0; i < m_netIFs.size(); i++)
   {
      if( i != 0 ) macaddresses += " ";
      macaddresses += m_netIFs[i]->getMACAddrString( true /* with separators */);
   }
   ipaddresses.clear();
   ipaddresses = m_address.generateIPString( false);
}
