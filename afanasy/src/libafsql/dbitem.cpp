#include "dbitem.h"

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
   std::string str = std::string("CREATE TABLE ") + v_dbGetTableName() + "\n(\n";
   if( v_dbGetIDsString().empty() == false )
      str += v_dbGetIDsString() + ",\n";
   for( int i = 0; i < dbAttributes.size(); i++)
   {
      if( i != 0 ) str += ",\n";
      str += "   " + dbAttributes[i]->createLine();
   }
   if( v_dbGetKeysString().empty() == false)
      str += ",\n " + v_dbGetKeysString();
   str += "\n)";

   queries->push_back( str);
}

void DBItem::v_dbInsert( std::list<std::string> * queries) const
{
   std::string str = std::string("INSERT INTO ") + v_dbGetTableName() + " (";
   for( int i = 0; i < dbAttributes.size(); i++)
   {
      if( i != 0 ) str += ",";
      str += dbAttributes[i]->getName();
   }
   str += ") VALUES (";
   for( int i = 0; i < dbAttributes.size(); i++)
   {
      if( i != 0 ) str += ",";
      str += dbAttributes[i]->getString();
   }
   str += ");";
   AFINFA("DBItem::dbInsert:\n%s", str.c_str())
   queries->push_back( str);
}

void DBItem::v_dbDelete( std::list<std::string> * queries) const
{
   queries->push_back( std::string("DELETE FROM ") + v_dbGetTableName()
                       + " WHERE " + dbAttributes.front()->getName() + "=" + dbAttributes.front()->getString() + ";");
}

void DBItem::v_dbUpdate( std::list<std::string> * queries, int attr) const
{
   std::string str = std::string("UPDATE ") + v_dbGetTableName() + " SET";
   bool attrfounded = false;
   int i = 0;
   for( int i = v_dbGetKeysNum(); i < dbAttributes.size(); i++)
   {
      if( attr > 0 )
      {
         if( dbAttributes[i]->getType() != attr ) continue;
      }
      else if( i != v_dbGetKeysNum()) str += ",";
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
   for( int i = 1; i < v_dbGetKeysNum(); i++)
      str += " AND " + dbAttributes[i]->getName() + "=" + dbAttributes[i]->getString();
   str += ";";
   AFINFA("DBItem::dbUpdate:\n%s", str.c_str())
   queries->push_back( str);
}

bool DBItem::v_dbSelect( PGconn * i_conn, const std::string * i_where)
{
    std::string query = "SELECT";
    for( int i = v_dbGetKeysNum(); i < dbAttributes.size(); i++)
    {
        if( i != v_dbGetKeysNum())
        {
            query += ",";
        }
        query += " " + dbAttributes[i]->getName();
    }
    query += " FROM " + v_dbGetTableName() + "\n WHERE ";
    if( i_where )
    {
        query += *i_where;
    }
    else
    {
        query += dbAttributes[0]->getName() + "=" + dbAttributes[0]->getString();
        for( int i = 1; i < v_dbGetKeysNum(); i++)
            query += " AND " + dbAttributes[i]->getName() + "=" + dbAttributes[i]->getString();
    }
    query += ";";
    PGresult * res = PQexec( i_conn, query.c_str());
    if( PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        AFERRAR("SQL Selecting node faild:%s\n%s", query.c_str(), PQerrorMessage( i_conn));
        return false;
    }

    int num_rows = PQntuples( res);
    AFINFA("DBItem::dbSelect: Returned query size=%d:\n%s", num_rows, query.c_str())

    if( num_rows != 1)
    {
        AFERRAR("DBItem::dbSelect: Not one (%d) item returned on query:\n%s", num_rows, query.c_str())
        return false;
    }

    int columns_size = PQnfields( res);

    if( columns_size != int( dbAttributes.size() - v_dbGetKeysNum()))
    {
        AFERRAR("DBItem::dbSelect: Invalid number of columns ( %d != %d ) returned on query:\n%s",
                columns_size, int( dbAttributes.size() - v_dbGetKeysNum()), query.c_str())
        return false;
    }

    int a = 0;
    for( int i = v_dbGetKeysNum(); i < dbAttributes.size(); i++, a++)
    {
        const char * value = PQgetvalue( res, 0, a);
        if( dbAttributes[i]->getType() <= DBAttr::_NUMERIC_END_)
        {
            dbAttributes[i]->set( af::stoi( value));
        }
        else
        {
            dbAttributes[i]->set( value);
        }
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
      cmd += v_dbGetTableName();
      cmd += " DROP COLUMN ";
      cmd += *it;
      std::cout << cmd << std::endl;
      queries->push_back( cmd);
   }

   // Check for new columns:
   for( int i = v_dbGetKeysNum(); i < dbAttributes.size(); i++)
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
      cmd += v_dbGetTableName();
      cmd += " ADD COLUMN ";
      cmd += dbAttributes[i]->createLine();
      std::cout << cmd << std::endl;
      queries->push_back( cmd);
   }
}

int DBItem::v_calcWeight() const
{
   int weight = sizeof(DBItem);
   for( int i = 0; i < dbAttributes.size(); i++)
   {
      weight += sizeof(dbAttributes[i]) + sizeof(*(dbAttributes[i]));
   }
   return weight;
}
