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

   void addJob( const af::Job * job, std::list<std::string> * queries);

   inline const std::string & v_dbGetTableName()  const { return TableName;}

private:
   std::string jobname;
   std::string blockname;
   std::string username;
   std::string hostname;
   std::string service;
   std::string description;
   std::string annotation;
   uint32_t tasksnum;
   uint32_t tasksdone;
   int64_t  taskssumruntime;
   int64_t  time_started;
   int64_t  time_done;

private:
   static const std::string TableName;
};
}
