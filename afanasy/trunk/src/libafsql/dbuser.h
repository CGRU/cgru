#pragma once

#include "../libafanasy/user.h"

#include "dbitem.h"

namespace afsql
{
class DBUser: public DBItem, public af::User
{
public:
   DBUser( const std::string & username, const std::string & host);
   DBUser( int uid = 0);
   virtual ~DBUser();

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
