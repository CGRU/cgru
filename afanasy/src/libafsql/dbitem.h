#pragma once

#include "name_afsql.h"

namespace afsql
{
class DBItem
{
public:
   DBItem();
   virtual ~DBItem();

   virtual const std::string & v_dbGetTableName() const = 0;

   void dbCreateTable(    std::list<std::string> * queries) const;
   void dbDropTable(      std::list<std::string> * queries) const;

   virtual void v_dbInsert( std::list<std::string> * queries) const;
   virtual void v_dbDelete( std::list<std::string> * queries) const;
   virtual void v_dbUpdate( std::list<std::string> * queries, int attr = -1) const;
   virtual bool v_dbSelect( PGconn * i_conn, const std::string * i_where = NULL);

   void dbUpdateTable( std::list<std::string> * queries, const std::list<std::string> & columns) const;

   virtual int v_calcWeight() const;

protected:
   virtual inline const std::string & v_dbGetIDsString()  const { return empty;}
   virtual inline const std::string & v_dbGetKeysString() const { return empty;}
   virtual inline       int           v_dbGetKeysNum()    const { return     0;}

   inline  void dbAddAttr( DBAttr * attr) { dbAttributes.push_back( attr);}

private:
   std::vector<DBAttr*> dbAttributes;
   static const std::string empty;
};
}
