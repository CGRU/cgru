#pragma once

#include "../libafanasy/user.h"

#include "dbitem.h"

namespace afsql
{
class DBUser: public DBItem, public af::User
{
public:
   DBUser( const std::string & username, const std::string & host);
   DBUser( int uid = 0);
   virtual ~DBUser();

   static const std::string dbGetIDsCmd();

   inline const std::string & v_dbGetTableName()  const { return TableName;}
   static void getIds(  std::list<int32_t> & uids, PGconn * i_conn);
   bool v_dbSelect( PGconn * i_conn, const std::string * i_where = NULL);

protected:
   inline const std::string & v_dbGetKeysString() const { return Keys;     }
   inline       int           v_dbGetKeysNum()    const { return KeysNum;  }

private:
   void addDBAttributes();

private:
// Unused by server, but used in vebvisor:
   std::string password;

private:
   static const std::string TableName;
   static const std::string Keys;
   static const int KeysNum;

};
}
