#pragma once

#include <QtCore/QVariant>

#include "dbitem.h"

#include "../libafanasy/taskprogress.h"

namespace afsql
{
class DBTaskProgress: public DBItem, public af::TaskProgress
{
public:
   DBTaskProgress();
   virtual ~DBTaskProgress();

   virtual int calcWeight() const;

   static const std::string dbPrepareInsert;
   static const std::string dbPrepareUpdate;

   inline const std::string & dbGetTableName() const { return TableName;}

   inline static const std::string dbWhereSelect( int id_job,  int id_block,  int id_task)
   {
//      return QString("id_job=%1 AND id_block=%2 AND id_task=%3").arg(id_job).arg(id_block).arg(id_task);
      return std::string("id_job=") + af::itos(id_job) + " AND id_block=" + af::itos(id_block) + " AND id_task=" + af::itos(id_task);
   }

   inline static void dbBindInsert( QSqlQuery *query, int id_job, int id_block, int id_task)
   {
      query->bindValue(":id_job",    QVariant( id_job   ));
      query->bindValue(":id_block",  QVariant( id_block ));
      query->bindValue(":id_task",   QVariant( id_task  ));
   }

   inline static void dbBindUpdate( QSqlQuery *query,
      uint32_t state,
      int      starts_count,
      int      errors_count,
      int64_t  time_start,
      int64_t  time_done,
      int      jobid,
      int      blocknum,
      int      tasknum)
{
   query->bindValue( 0, state);
   query->bindValue( 1, starts_count);
   query->bindValue( 2, errors_count);
   query->bindValue( 3, qlonglong( time_start));
   query->bindValue( 4, qlonglong( time_done));
   query->bindValue( 5, jobid);
   query->bindValue( 6, blocknum);
   query->bindValue( 7, tasknum);
}

protected:
   inline const std::string & dbGetIDsString()  const { return IDs;      }
   inline const std::string & dbGetKeysString() const { return Keys;     }

private:
   static const std::string TableName;
   static const std::string IDs;
   static const std::string Keys;
};
}
