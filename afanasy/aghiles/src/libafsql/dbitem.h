#pragma once

#include "name_afsql.h"

class QSqlDatabase;

namespace afsql
{
class DBItem
{
public:
   DBItem();
   virtual ~DBItem();

   virtual const std::string & dbGetTableName() const = 0;

   void dbCreateTable(    std::list<std::string> * queries) const;
   void dbDropTable(      std::list<std::string> * queries) const;

   virtual void dbInsert( std::list<std::string> * queries) const;
   virtual void dbDelete( std::list<std::string> * queries) const;
   virtual void dbUpdate( std::list<std::string> * queries, int attr = -1) const;
   virtual bool dbSelect( QSqlDatabase * db, const std::string * where = NULL);

   void dbUpdateTable( std::list<std::string> * queries, const std::list<std::string> & columns) const;

   virtual int calcWeight() const;

protected:
   virtual inline const std::string & dbGetIDsString()  const { return empty;}
   virtual inline const std::string & dbGetKeysString() const { return empty;}
   virtual inline       int           dbGetKeysNum()    const { return     0;}

   inline  void dbAddAttr( DBAttr * attr) { dbAttributes.push_back( attr);}

private:
   std::vector<DBAttr*> dbAttributes;
   static const std::string empty;
};
}
