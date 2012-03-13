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

   static const QString dbPrepareInsert;
   static const QString dbPrepareUpdate;

   inline static const QString dbWhereSelect( int id_job,  int id_block,  int id_task)
   {
      return QString("id_job=%1 AND id_block=%2 AND id_task=%3").arg(id_job).arg(id_block).arg(id_task);
   }

   inline static void dbBindInsert( QSqlQuery *query, const QVariant & id_job, const QVariant & id_block, const QVariant & id_task)
   {
      query->bindValue(":id_job",    id_job   );
      query->bindValue(":id_block",  id_block );
      query->bindValue(":id_task",   id_task  );
   }

   inline static void dbBindUpdate( QSqlQuery *query,
      uint32_t state,
      int      starts_count,
      int      errors_count,
      uint32_t time_start,
      uint32_t time_done,
      int      jobid,
      int      blocknum,
      int      tasknum)
{
   query->bindValue( 0, state);
   query->bindValue( 1, starts_count);
   query->bindValue( 2, errors_count);
   query->bindValue( 3, time_start);
   query->bindValue( 4, time_done);
   query->bindValue( 5, jobid);
   query->bindValue( 6, blocknum);
   query->bindValue( 7, tasknum);
}

protected:
   inline const QString & dbGetTableName() const { return TableName;}
   inline const QString & dbGetIDs()       const { return IDs;      }
   inline const QString & dbGetKeys()      const { return Keys;     }

private:
   static const QString TableName;
   static const QString IDs;
   static const QString Keys;
};
}
