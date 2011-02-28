#include "dbitem.h"

#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlerror.h>
#include <QtSql/qsqlquery.h>

#include "dbattr.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afsql;

const QString DBItem::empty;

DBItem::DBItem()
{
}

DBItem::~DBItem()
{
   for( int i = 0; i < dbAttributes.size(); i++) delete dbAttributes[i];
}

void DBItem::dbDropTable( QSqlDatabase * db) const
{
   if( db->isOpen() == false )
   {
      AFERROR("DBItem::dbDropTable: Database connection is not open\n");
      return;
   }
   QStringList tables = db->tables();
   if( tables.contains( dbGetTableName()))
   {
      QSqlQuery q( *db);
      QString str = QString("DROP TABLE %1;").arg( dbGetTableName());
      AFINFA("DBItem::dbDropTable: executing query:\n%s\n", str.toUtf8().data());
      q.exec( str);
      qChkErr(q, QString("DBItem::dbDropTable %1:\n").arg( dbGetTableName()));
   }
   else
   {
      AFINFA("DBItem::dbDropTable: no table '%s' founded.\n", dbGetTableName().toUtf8().data());
   }
}

void DBItem::dbCreateTable( QSqlDatabase * db) const
{
   if( db->isOpen() == false )
   {
      AFERROR("DBItem::dbCreateTable: Database connection is not open\n");
      return;
   }
   QStringList tables = db->tables();
   if( tables.contains( dbGetTableName()))
   {
      AFERRAR("DBItem::dbCreateTable: Table '%s' already exists.\n", dbGetTableName().toUtf8().data());
      return;
   }

   QString str(QString("CREATE TABLE %1\n(\n").arg( dbGetTableName()));
   if( dbGetIDs().isEmpty() == false) str += QString("%1,\n").arg( dbGetIDs());
   for( int i = 0; i < dbAttributes.size(); i++)
   {
      if( i != 0 ) str += ",\n";
      str += QString("   %1").arg( dbAttributes[i]->createLine());
   }
   if( dbGetKeys().isEmpty() == false)
      str += QString(",\n %1").arg( dbGetKeys());
   str += "\n)";

   QSqlQuery q( *db);
   AFINFA("DBItem::dbCreateTable: executing query:\n%s\n", str.toUtf8().data());
   q.exec( str);
   qChkErr(q, QString("DBItem::dbCreateTable %1:\n").arg( dbGetTableName()));
}

void DBItem::dbInsert( QStringList  * queries) const
{
   QString str = QString("INSERT INTO %1 (").arg( dbGetTableName());
   for( int i = 0; i < dbAttributes.size(); i++)
   {
      if( i != 0 ) str += ",\n";
      str += dbAttributes[i]->getName();
   }
   str += "\n) VALUES (";
   for( int i = 0; i < dbAttributes.size(); i++)
   {
      if( i != 0 ) str += ",";
      str += dbAttributes[i]->getString();
   }
   str += ");";
AFINFA("DBItem::dbInsert:\n%s\n", str.toUtf8().data());
   *queries << str;
}

void DBItem::dbDelete( QStringList  * queries) const
{
  *queries << QString("DELETE FROM %1 WHERE %2=%3;")
         .arg( dbGetTableName()).arg( dbAttributes[0]->getName()).arg( dbAttributes[0]->getString());
}

void DBItem::dbUpdate( QStringList  * queries, int attr) const
{
   QString str = QString("UPDATE %1 SET").arg( dbGetTableName());
   bool attrfounded = false;
   for( int i = dbGetKeysNum(); i < dbAttributes.size(); i++)
   {
      if( attr > 0 )
      {
         if( dbAttributes[i]->getType() != attr ) continue;
      }
      else if( i != dbGetKeysNum()) str += ",";
      str += QString(" %1=%2").arg( dbAttributes[i]->getName()).arg( dbAttributes[i]->getString());
      if( attr > 0 )
      {
         attrfounded = true;
         break;
      }
   }
   if(( attr > 0 ) && ( false == attrfounded ))
   {
      AFERRAR("DBItem::dbUpdate: attr=%d not founded.\n", attr);
      return;
   }
   str += QString(" WHERE %1=%2").arg( dbAttributes[0]->getName()).arg( dbAttributes[0]->getString());
   for( int i = 1; i < dbGetKeysNum(); i++)
      str += QString(" AND %1=%2").arg( dbAttributes[i]->getName()).arg( dbAttributes[i]->getString());
   str += ";";
AFINFA("DBItem::dbUpdate:\n%s\n", str.toUtf8().data());
   *queries << str;
}

bool DBItem::dbSelect( QSqlDatabase * db, const QString * where)
{
   QSqlQuery q( *db);
   QString str = QString("SELECT");
   for( int i = dbGetKeysNum(); i < dbAttributes.size(); i++)
   {
      if( i != dbGetKeysNum()) str += ",";
      str += QString(" %1").arg( dbAttributes[i]->getName());
   }
   str += QString(" FROM %1\n WHERE ").arg( dbGetTableName());
   if( where )
      str += *where;
   else
   {
      str += QString("%1=%2").arg( dbAttributes[0]->getName()).arg( dbAttributes[0]->getString());
      for( int i = 1; i < dbGetKeysNum(); i++)
         str += QString(" AND %1=%2").arg( dbAttributes[i]->getName()).arg( dbAttributes[i]->getString());
   }
   str += ";";
   q.exec( str);
   AFINFA("DBItem::dbSelect: Returned query size=%d:\n%s\n", q.size(), str.toUtf8().data())
   if( q.size() != 1)
   {
      AFERRAR("DBItem::dbSelect: Not one (%d) item returned on query:\n%s\n", q.size(), str.toUtf8().data())
      return false;
   }
   q.next();
   int a = 0;
   for( int i = dbGetKeysNum(); i < dbAttributes.size(); i++, a++)
      dbAttributes[i]->set( q.value(a));

   return true;
}

int DBItem::calcWeight() const
{
   int weight = sizeof(DBItem);
   for( int i = 0; i < dbAttributes.size(); i++)
   {
      weight += sizeof(dbAttributes[i]) + sizeof(*(dbAttributes[i]));
   }
   return weight;
}
