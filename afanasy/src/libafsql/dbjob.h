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

   static const std::string dbGetIDsCmd();

   bool dbAdd( PGconn * i_conn) const;

   virtual bool v_dbSelect( PGconn * i_conn, const std::string * i_where = NULL);
   virtual void v_dbDelete( std::list<std::string> * queries) const;
   virtual void dbDeleteNoStatistics( std::list<std::string> * queries) const;

   inline const std::string & v_dbGetTableName()  const { return TableName;}

protected:
   inline const std::string & v_dbGetKeysString() const { return Keys;     }
   inline       int           v_dbGetKeysNum()    const { return KeysNum;  }

protected:
   DBJobProgress * progress;    ///< Tasks progress.

private:
   void addDBAttributes();
   virtual af::BlockData * v_newBlockData( af::Msg * msg);
   virtual af::BlockData * v_newBlockData( const JSON & i_object, int i_num);

private:
   static const std::string TableName;
   static const std::string Keys;
   static const int KeysNum;
   static DBStatistics statistics;
};
}
