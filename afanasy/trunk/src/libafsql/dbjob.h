#pragma once

#include "../libafanasy/job.h"

#include "name_afsql.h"
#include "dbitem.h"
#include "dbstatistics.h"

namespace afsql
{
class DBJob: public DBItem, public af::Job
{
public:
   DBJob( int Id = 0);
   virtual ~DBJob();

   static void getIds(  std::list<int32_t> & uids, QSqlDatabase * db);

   void dbAdd( QSqlDatabase * db) const;

   virtual bool dbSelect( QSqlDatabase * db, const QString * where = NULL);
   virtual void dbDelete( QStringList  * queries) const;
   virtual void dbDeleteNoStatistics( QStringList  * queries) const;

protected:
   inline const QString & dbGetTableName()  const { return TableName;}
   inline const QString & dbGetKeys()       const { return Keys;     }
   inline       int       dbGetKeysNum()    const { return KeysNum;  }

protected:
   DBJobProgress * progress;    ///< Tasks progress.

private:
   void addDBAttributes();
   virtual af::BlockData * newBlockData( af::Msg * msg);

private:
   static const QString TableName;
   static const QString Keys;
   static const int KeysNum;
   static DBStatistics statistics;
};
}
