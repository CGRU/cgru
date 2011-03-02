#pragma once

#include "../libafanasy/name_af.h"

#include "dbitem.h"

namespace afsql
{
class DBStatistics: public DBItem
{
public:
   DBStatistics();
   virtual ~DBStatistics();

   void addJob( const af::Job * job, QStringList * queries);

protected:
   inline const QString & dbGetTableName()  const { return TableName;}

private:
   QString jobname;
   QString blockname;
   QString username;
   QString hostname;
   QString service;
   std::string description;
   std::string annotation;
   uint32_t time_started;
   uint32_t time_done;
   uint32_t tasksnum;
   uint32_t tasksdone;
   uint32_t taskssumruntime;

private:
   static const QString TableName;
};
}
