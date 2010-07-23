#pragma once

#include <QtCore/qvariant.h>

#include "../libafanasy/taskdata.h"

#include "dbitem.h"

namespace afsql
{
class DBTaskData: public DBItem, public af::TaskData
{
public:
   DBTaskData();
   DBTaskData( af::Msg * msg);
   virtual ~DBTaskData();

   static const QString dbPrepareInsert;
   static const QString dbWhereSelect( int id_job, int id_block, int id_task);

   void dbBindInsert( QSqlQuery *query, const QVariant & id_job, const QVariant & id_block, const QVariant & id_task) const;

protected:
   inline const QString & dbGetTableName() const { return TableName;}
   inline const QString & dbGetIDs()       const { return IDs;      }
   inline const QString & dbGetKeys()      const { return Keys;     }

protected:
   virtual void readwrite( af::Msg * msg);

private:
   static const QString TableName;
   static const QString Keys;
   static const QString IDs;

private:
   void addDBAttributes();
};
}
