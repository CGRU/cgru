#pragma once

#include "../libafanasy/render.h"

#include "dbitem.h"

namespace afsql
{
class DBRender: public DBItem, public af::Render
{
public:
   DBRender( int Id = 0);
   DBRender( af::Msg * msg);

   virtual ~DBRender();

   static const std::string dbGetIDsCmd();

   inline const std::string & v_dbGetTableName()  const { return TableName;}
   static void getIds(  std::list<int32_t> & uids, PGconn * i_conn);

   virtual void v_dbInsert( std::list<std::string> * queries) const;
   virtual void v_dbUpdate( std::list<std::string> * queries, int attr = -1) const;
   virtual bool v_dbSelect( PGconn * i_conn, const std::string * i_where = NULL);

protected:
   inline const std::string & v_dbGetKeysString() const { return Keys;     }
   inline       int           v_dbGetKeysNum()    const { return KeysNum;  }

private:
   void addDBAttributes();
   void updateNetIFs() const;

private:
   static const std::string TableName;
   static const std::string Keys;
   static const int KeysNum;

   mutable std::string ipaddresses;
   mutable std::string macaddresses;
};
}
