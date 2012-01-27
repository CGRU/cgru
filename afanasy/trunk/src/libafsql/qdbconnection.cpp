#include "qdbconnection.h"

#include <stdio.h>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>

#include "../libafanasy/environment.h"

#include "dbjob.h"
#include "dbblockdata.h"
#include "dbrender.h"
#include "dbuser.h"
#include "dbstatistics.h"
#include "dbtaskdata.h"
#include "dbtaskprogress.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afsql;

DBConnection::DBConnection( const std::string & connection_name):
   name( connection_name),
   working( false)
{
   db = newDatabase( name);
   working = true;
   working = DBOpen();
   DBClose();
}

DBConnection::~DBConnection()
{
   db->close();
   delete db;
   QSqlDatabase::removeDatabase( name.c_str());
}

bool DBConnection::DBOpen()
{
   if( working == false ) return false;
   AFINFO("Trying to lock DB...")

   m_mutex.Lock();

   AFINFO(" - Done")

   if( db->isOpen())
   {
      AFERROR("DBConnection::DBOpen: database is already open:")

        m_mutex.Unlock();

      return false;
   }

   afsql::setDatabase( db);
   if( db->open() == false )
   {
      AFERROR("DBConnection::DBOpen: UNABLE TO OPEN DATABASE:")
      std::cout << afsql::qtos(db->lastError().text()) << std::endl;

      m_mutex.Unlock();

      return false;
   }
   else return true;
}

void DBConnection::DBClose()
{
    db->close();

    m_mutex.Unlock();

    AFINFO("DB Unlocked.")
}

void DBConnection::dropAllTables()
{
   if( working == false ) return;
   if( db->isOpen() == false )
   {
      AFERROR("DBConnection::dropAllTables: Database connection is not open.")
      return;
   }
   QStringList tables = db->tables();
   for( int i = 0; i < tables.size(); i++)
   {
      QString query = QString("DROP TABLE %1 CASCADE;").arg( tables[i]);
      std::cout << afsql::qtos( query) << std::endl;
      QSqlQuery q( query, *db);
      q.exec();
      //qChkErr( q, "DBConnection::dropAllTables:");
   }
}

bool DBConnection::getItem( DBItem * item)
{
   if( working == false ) return false;
   return item->dbSelect( db);
}

void DBConnection::addJob( DBJob * job)
{
   if( working == false ) return;
   job->dbAdd( db);
}

void DBConnection::execute( const std::list<std::string> * queries)
{
   if( working == false ) return;
   AFINFO("Executing queries:")
   for( std::list<std::string>::const_iterator it = queries->begin(); it != queries->end(); it++)
   {
      QSqlQuery q( afsql::stoq(*it), *db);
      q.exec();
#ifdef AFOUTPUT
printf("%s\n", (*it).c_str());
#endif
   }
}

const std::list<int> DBConnection::getIntegers( const std::string & query)
{
   std::list<int> intlinst;
   if( working == false ) return intlinst;
   if( db->isOpen() == false )
   {
      AFERROR("DBConnection::getIntegers: Database connection is not open.")
      return intlinst;
   }
   QSqlQuery q( *db);
   q.exec( afsql::stoq( query));
#ifdef AFOUTPUT
printf("%s\n", query.c_str());
#endif
   while( q.next()) intlinst.push_back( q.value(0).toInt());
   return intlinst;
}

void DBConnection::dropTable( const std::string & tableName)
{
   if( db->isOpen() == false )
   {
      AFERROR("DBConnection::dropTable: Database connection is not open.")
      return;
   }
   QStringList tables = db->tables();
   if( tables.contains( afsql::stoq( tableName)))
   {
      QSqlQuery q( *db);
      std::string str = std::string("DROP TABLE ") + tableName + ";";
      AFINFA("DBConnection::dropTable: executing query:\n%s", str.c_str())
      q.exec( afsql::stoq(str));
      qChkErr(q, std::string("DBItem::dbDropTable: ") + tableName );
   }
   else
   {
      printf("DBConnection::dropTable: no table \"%s\" founded.", tableName .c_str());
   }
}

const std::list<std::string> DBConnection::getTableColumnsNames( const std::string & tableName)
{
   std::list<std::string> columns;
   if( db->isOpen() == false )
   {
      AFERROR("DBConnection::getTableColumnsNames: Database connection is not open.")
      return columns;
   }
   QSqlQuery q( *db);
   q.exec( QString("SELECT * FROM ") + afsql::stoq(tableName));

   if( qChkErr(q, std::string("DBItem::getTableColumnsNames: ") + tableName )) return columns;

   QSqlRecord qrecord = q.record();
   for( int i = 0; i < qrecord.count(); i++)
   {
      QString fieldname = qrecord.fieldName( i);
      if( false == fieldname.isEmpty())
         columns.push_back( afsql::qtos( fieldname));
   }
}
