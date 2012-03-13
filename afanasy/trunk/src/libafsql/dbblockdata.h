#pragma once

#include "../libafanasy/blockdata.h"

#include "dbitem.h"

namespace afsql
{
class DBBlockData: public DBItem, public af::BlockData
{
public:
   DBBlockData();
   DBBlockData( af::Msg * msg);
   DBBlockData( int BlockNum, int JobId);
   virtual ~DBBlockData();

   void dbAdd( QSqlDatabase * db) const;
   virtual bool dbSelect( QSqlDatabase * db, const QString * where = NULL);

protected:
   inline const QString & dbGetTableName()  const { return TableName;}
   inline const QString & dbGetKeys()       const { return Keys;     }
   inline       int       dbGetKeysNum()    const { return KeysNum;  }

private:
   void addDBAttributes();
   virtual af::TaskData * createTask( af::Msg * msg);

private:
   static const QString TableName;
   static const QString Keys;
   static const int KeysNum;
};
}
