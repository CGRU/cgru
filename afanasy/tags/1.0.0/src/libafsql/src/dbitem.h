#pragma once

#include <QtCore/QList>
#include <QtCore/QStringList>

#include "name_afsql.h"

class QSqlDatabase;

namespace afsql
{
class DBItem
{
public:
   DBItem();
   ~DBItem();

   void dbDropTable(      QSqlDatabase * db     ) const;
   void dbCreateTable(    QSqlDatabase * db     ) const;
   virtual void dbInsert( QStringList  * queries) const;
   virtual void dbDelete( QStringList  * queries) const;
   virtual void dbUpdate( QStringList  * queries, int attr = -1) const;
   virtual bool dbSelect( QSqlDatabase * db, const QString * where = NULL);

   virtual int calcWeight() const;

protected:
   virtual const QString & dbGetTableName() const = 0;
   virtual const QString & dbGetIDs()       const { return empty;}
   virtual const QString & dbGetKeys()      const { return empty;}
   virtual       int       dbGetKeysNum()   const { return 0;}
   inline  void dbAddAttr( DBAttr * attr) { dbAttributes.append( attr);}

private:
   QList<DBAttr*> dbAttributes;
   static const QString empty;
};
}
