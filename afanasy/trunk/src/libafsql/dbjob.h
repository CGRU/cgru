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

   void dbAdd( QSqlDatabase * db) const;

   virtual bool dbSelect( QSqlDatabase * db, const std::string * where = NULL);
   virtual void dbDelete( std::list<std::string> * queries) const;
   virtual void dbDeleteNoStatistics( std::list<std::string> * queries) const;

   inline const std::string & dbGetTableName()  const { return TableName;}

protected:
   inline const std::string & dbGetKeysString() const { return Keys;     }
   inline       int           dbGetKeysNum()    const { return KeysNum;  }

protected:
   DBJobProgress * progress;    ///< Tasks progress.

private:
   void addDBAttributes();
   virtual af::BlockData * newBlockData( af::Msg * msg);

private:
   static const std::string TableName;
   static const std::string Keys;
   static const int KeysNum;
   static DBStatistics statistics;
};
}
