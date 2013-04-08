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
   DBBlockData( const JSON & i_object, int i_num);
   DBBlockData( int BlockNum, int JobId);
   virtual ~DBBlockData();

   inline const std::string & v_dbGetTableName()  const { return TableName;}
   bool dbAdd( PGconn * i_conn) const;
   virtual bool v_dbSelect( PGconn * i_conn, const std::string * i_where = NULL);

protected:
   inline const std::string & v_dbGetKeysString() const { return Keys;     }
   inline       int           v_dbGetKeysNum()    const { return KeysNum;  }

private:
   void addDBAttributes();
   virtual af::TaskData * createTask( af::Msg * msg);
   virtual af::TaskData * createTask( const JSON & i_object);

private:
   static const std::string TableName;
   static const std::string Keys;
   static const int KeysNum;
};
}
