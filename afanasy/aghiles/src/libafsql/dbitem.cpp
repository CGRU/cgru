#include "dbitem.h"

#include <QtCore/QStringList>
#include <QtCore/QVariant>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include "dbattr.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afsql;

const std::string DBItem::empty;

DBItem::DBItem()
{
}

DBItem::~DBItem()
{
   for( int i = 0; i < dbAttributes.size(); i++) delete dbAttributes[i];
}

void DBItem::dbCreateTable( std::list<std::string> * queries) const
{
   std::string str = std::string("CREATE TABLE ") + dbGetTableName() + "\n(\n";
   if( dbGetIDsString().empty() == false )
      str += dbGetIDsString() + ",\n";
   for( int i = 0; i < dbAttributes.size(); i++)
   {
      if( i != 0 ) str += ",\n";
      str += "   " + dbAttributes[i]->createLine();
   }
   if( dbGetKeysString().empty() == false)
      str += ",\n " + dbGetKeysString();
   str += "\n)";

   queries->push_back( str);
}

void DBItem::dbInsert( std::list<std::string> * queries) const
{
   std::string str = std::string("INSERT INTO ") + dbGetTableName() + " (";
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
   AFINFA("DBItem::dbInsert:\n%s", str.c_str())
   queries->push_back( str);
}

void DBItem::dbDelete( std::list<std::string> * queries) const
{
   queries->push_back( std::string("DELETE FROM ") + dbGetTableName()
                       + " WHERE " + dbAttributes.front()->getName() + "=" + dbAttributes.front()->getString() + ";");
}

void DBItem::dbUpdate( std::list<std::string> * queries, int attr) const
{
   std::string str = std::string("UPDATE ") + dbGetTableName() + " SET";
   bool attrfounded = false;
   int i = 0;
   for( int i = dbGetKeysNum(); i < dbAttributes.size(); i++)
   {
      if( attr > 0 )
      {
         if( dbAttributes[i]->getType() != attr ) continue;
      }
      else if( i != dbGetKeysNum()) str += ",";
      str += " " + dbAttributes[i]->getName() + "=" + dbAttributes[i]->getString();
      if( attr > 0 )
      {
         attrfounded = true;
         break;
      }
   }
   if(( attr > 0 ) && ( false == attrfounded ))
   {
      AFERRAR("DBItem::dbUpdate: attr=%d not founded.", attr)
      return;
   }
   str += " WHERE " + dbAttributes[0]->getName() + "=" + dbAttributes[0]->getString();
   for( int i = 1; i < dbGetKeysNum(); i++)
      str += " AND " + dbAttributes[i]->getName() + "=" + dbAttributes[i]->getString();
   str += ";";
   AFINFA("DBItem::dbUpdate:\n%s", str.c_str())
   queries->push_back( str);
}

bool DBItem::dbSelect( QSqlDatabase * db, const std::string * where)
{
   QSqlQuery q( *db);
   std::string str = "SELECT";
   for( int i = dbGetKeysNum(); i < dbAttributes.size(); i++)
   {
      if( i != dbGetKeysNum()) str += ",";
      str += " " + dbAttributes[i]->getName();
   }
   str += " FROM " + dbGetTableName() + "\n WHERE ";
   if( where )
      str += *where;
   else
   {
      str += dbAttributes[0]->getName() + "=" + dbAttributes[0]->getString();
      for( int i = 1; i < dbGetKeysNum(); i++)
         str += " AND " + dbAttributes[i]->getName() + "=" + dbAttributes[i]->getString();
   }
   str += ";";
   q.exec( afsql::stoq( str));
   AFINFA("DBItem::dbSelect: Returned query size=%d:\n%s", q.size(), str.c_str())
   if( q.size() != 1)
   {
      AFERRAR("DBItem::dbSelect: Not one (%d) item returned on query:\n%s", q.size(), str.c_str())
      return false;
   }
   q.next();
   int a = 0;
   for( int i = dbGetKeysNum(); i < dbAttributes.size(); i++, a++)
   {
      if( dbAttributes[i]->getType() <= DBAttr::_NUMERIC_END_)
         dbAttributes[i]->set( q.value(a).toLongLong());
      else
         dbAttributes[i]->set( afsql::qtos( q.value(a).toString()));
   }
   return true;
}

void DBItem::dbUpdateTable( std::list<std::string> * queries, const std::list<std::string> & columns) const
{
   // Check for spare columns:
   for( std::list<std::string>::const_iterator it = columns.begin(); it != columns.end(); it++)
   {
      if( (*it).find("id_") == 0 ) continue;
      bool column_exists = false;
      for( int i = 0; i < dbAttributes.size(); i++)
      {
         if( *it == dbAttributes[i]->getName())
         {
            column_exists = true;
            break;
         }
      }
      if( column_exists ) continue;

      std::string cmd("ALTER TABLE ");
      cmd += dbGetTableName();
      cmd += " DROP COLUMN ";
      cmd += *it;
      std::cout << cmd << std::endl;
      queries->push_back( cmd);
   }

   // Check for new columns:
   for( int i = dbGetKeysNum(); i < dbAttributes.size(); i++)
   {
      bool column_exists = false;
      for( std::list<std::string>::const_iterator it = columns.begin(); it != columns.end(); it++)
      {
         if( *it == dbAttributes[i]->getName())
         {
            column_exists = true;
            break;
         }
      }
      if( column_exists ) continue;

      std::string cmd("ALTER TABLE ");
      cmd += dbGetTableName();
      cmd += " ADD COLUMN ";
      cmd += dbAttributes[i]->createLine();
      std::cout << cmd << std::endl;
      queries->push_back( cmd);
   }
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
