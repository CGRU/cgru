#pragma once

#include "dbitem.h"

#include "../libafanasy/taskprogress.h"

namespace afsql
{
class DBTaskProgress: public DBItem, public af::TaskProgress
{
public:
   DBTaskProgress();
   virtual ~DBTaskProgress();

   virtual int v_calcWeight() const;

   inline const std::string & v_dbGetTableName() const { return TableName;}

   inline static const std::string dbWhereSelect( int id_job,  int id_block,  int id_task)
   {
      return std::string("id_job=") + af::itos(id_job) + " AND id_block=" + af::itos(id_block) + " AND id_task=" + af::itos(id_task);
   }

   static bool dbPrepareInsert( PGconn * i_conn);
   static bool dbPrepareInsertExec(  int i_id_job, int i_id_block, int i_id_task, PGconn * i_conn);

protected:
   inline const std::string & v_dbGetIDsString()  const { return IDs;      }
   inline const std::string & v_dbGetKeysString() const { return Keys;     }

private:
   static const std::string TableName;
   static const std::string IDs;
   static const std::string Keys;

   static char ms_db_prepare_name[];
};
}
