#pragma once

#include "../libafanasy/name_af.h"

#include "dbitem.h"
#include "dbstatblock.h"

namespace afsql
{
class DBStatJob: public DBItem
{
public:
   DBStatJob();
   virtual ~DBStatJob();

   void dbDropTable(      QSqlDatabase * db     ) const;
   void dbCreateTable(    QSqlDatabase * db     ) const;
   void addJob( const af::Job * job, QStringList * queries);

protected:
   inline const QString & dbGetTableName()  const { return TableName;}

private:
   QString name;
   QString description;
   QString username;
   QString hostname;
   uint32_t flags;
   uint32_t time_started;
   uint32_t time_done;
   uint32_t taskssumruntime;

   DBStatBlock statblock;

private:
   static const QString TableName;
};
}
