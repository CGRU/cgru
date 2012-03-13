#pragma once

#include "../libafanasy/name_af.h"

#include "dbitem.h"

namespace afsql
{
class DBStatBlock: public DBItem
{
public:
   DBStatBlock();
   virtual ~DBStatBlock();

   void addBlocks( const af::Job * job, QStringList * queries);

protected:
   inline const QString & dbGetTableName()  const { return TableName;}

private:
   QString name;
   QString name_job;
   QString username;
   QString taskstype;
   uint32_t flags;
   uint32_t tasksnum;
   uint32_t tasksdone;
   uint32_t taskssumruntime;

private:
   static const QString TableName;
};
}
