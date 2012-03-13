#pragma once

#include "../libafanasy/render.h"

#include "dbitem.h"

namespace afsql
{
class DBRender: public DBItem, public af::Render
{
public:
   DBRender( int Id = 0);
   DBRender( af::Msg * msg, const af::Address * addr = NULL );

   virtual ~DBRender();

   static void getIds(  std::list<int32_t> & uids, QSqlDatabase * db);
   bool dbSelect( QSqlDatabase * db, const QString * where = NULL);

protected:
   inline const QString & dbGetTableName()  const { return TableName;}
   inline const QString & dbGetKeys()       const { return Keys;     }
   inline       int       dbGetKeysNum()    const { return KeysNum;  }

private:
   static const QString TableName;
   static const QString Keys;
   static const int KeysNum;
   void addDBAttributes();
};
}
