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

   inline const std::string & dbGetTableName()  const { return TableName;}
   static void getIds(  std::list<int32_t> & uids, QSqlDatabase * db);

   virtual void dbInsert( std::list<std::string> * queries) const;
   virtual void dbUpdate( std::list<std::string> * queries, int attr = -1) const;
   virtual bool dbSelect( QSqlDatabase * db, const std::string * where = NULL);

protected:
   inline const std::string & dbGetKeysString() const { return Keys;     }
   inline       int           dbGetKeysNum()    const { return KeysNum;  }

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
