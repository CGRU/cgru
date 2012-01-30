#pragma once

#include "../libafanasy/blockdata.h"

#include "dbitem.h"

namespace afsql
{
class DBBlockData: public DBItem, public af::BlockData
{
public:
   DBBlockData();
   DBBlockData( af::Msg * msg);
   DBBlockData( int BlockNum, int JobId);
   virtual ~DBBlockData();

   inline const std::string & dbGetTableName()  const { return TableName;}
   bool dbAdd( PGconn * i_conn) const;
   virtual bool dbSelect( PGconn * i_conn, const std::string * i_where = NULL);

protected:
   inline const std::string & dbGetKeysString() const { return Keys;     }
   inline       int           dbGetKeysNum()    const { return KeysNum;  }

private:
   void addDBAttributes();
   virtual af::TaskData * createTask( af::Msg * msg);

private:
   static const std::string TableName;
   static const std::string Keys;
   static const int KeysNum;
};
}
