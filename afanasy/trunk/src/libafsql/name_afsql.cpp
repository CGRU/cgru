#include "name_afsql.h"

#include <QtSql/qsqlerror.h>

#include <stdio.h>

#include "../libafanasy/environment.h"
#include "../libafanasy/job.h"
#include "../libafanasy/blockdata.h"
#include "../libafanasy/user.h"
#include "../libafanasy/render.h"

#include "dbattr.h"
#include "dbblockdata.h"
#include "dbjob.h"
#include "dbrender.h"
#include "dbstatistics.h"
#include "dbtaskdata.h"
#include "dbtaskprogress.h"
#include "dbuser.h"
#include "qdbconnection.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

void afsql::init()
{
   DBAttr::init();
}

bool afsql::qChkErr( const QSqlQuery & q, const std::string & str)
{
   if( q.lastError().isValid())
   {
      AFERRAR("%s: Query check:", str.c_str())
      std::cout << afsql::qtos( q.lastError().databaseText()) << std::endl;
      return true;
   }
   return false;
}

const std::string afsql::qtos( const QString & str ){ return std::string( str.toUtf8().data()); }
const QString afsql::stoq( const std::string & str ){ return QString::fromUtf8(str.c_str());    }

QSqlDatabase * afsql::newDatabase( const std::string & connection_name)
{
#ifdef AFOUTPUT
printf("Trying to create new DB connection to \"%s\" as \"%s\"\n", af::Environment::get_DB_Type().c_str(), connection_name.c_str());
#endif
   QSqlDatabase * db = new QSqlDatabase( QSqlDatabase::addDatabase( afsql::stoq( af::Environment::get_DB_Type()),
                                                                    afsql::stoq( connection_name)));
   setDatabase( db);
   return db;
}

void afsql::setDatabase( QSqlDatabase * db)
{
#ifdef AFOUTPUT
printf("Trying to setup DB connection \"%s\"\n", afsql::qtos( db->connectionName()).c_str());
printf(" host=\"%s\"",        af::Environment::get_DB_HostName().c_str());
printf(", database=\"%s\"",   af::Environment::get_DB_DataBaseName().c_str());
printf(", user=\"%s\"",       af::Environment::get_DB_UserName().c_str());
printf(", passwd=\"%s\"",     af::Environment::get_DB_Password().c_str());
printf("\n");
#endif
   db->setHostName(      afsql::stoq( af::Environment::get_DB_HostName()      ));
   db->setDatabaseName(  afsql::stoq( af::Environment::get_DB_DataBaseName()  ));
   db->setUserName(      afsql::stoq( af::Environment::get_DB_UserName()      ));
   db->setPassword(      afsql::stoq( af::Environment::get_DB_Password()      ));
}

void afsql::ResetUsers( DBConnection * dbconnenction)
{
   if( dbconnenction->isWorking() == false ) return;
   if( dbconnenction->isOpen()    == false )
   {
      AFERROR("DBConnection::ResetUsers: Database connection is not open")
      return;
   }
   DBUser user;
   dbconnenction->dropTable( user.dbGetTableName());
   std::list<std::string> queries;
   user.dbCreateTable( &queries);
   dbconnenction->execute( &queries);
}

void afsql::ResetRenders( DBConnection * dbconnenction)
{
   if( dbconnenction->isWorking() == false ) return;
   if( dbconnenction->isOpen()    == false )
   {
      AFERROR("DBConnection::ResetRenders: Database connection is not open")
      return;
   }
   DBRender render;
   dbconnenction->dropTable( render.dbGetTableName());
   std::list<std::string> queries;
   render.dbCreateTable( &queries);
   dbconnenction->execute( &queries);
}

void afsql::ResetJobs( DBConnection * dbconnenction)
{
   if( dbconnenction->isWorking() == false ) return;
   if( dbconnenction->isOpen()    == false )
   {
      AFERROR("DBConnection::ResetOnline: Database connection is not open")
      return;
   }

   DBJob job;
   DBBlockData block;
   DBTaskData task;
   DBTaskProgress progress;

   dbconnenction->dropTable( progress.dbGetTableName());
   dbconnenction->dropTable( task.dbGetTableName());
   dbconnenction->dropTable( block.dbGetTableName());
   dbconnenction->dropTable( job.dbGetTableName());

   std::list<std::string> queries;
   job.dbCreateTable( &queries);
   block.dbCreateTable( &queries);
   task.dbCreateTable( &queries);
   progress.dbCreateTable( &queries);
   dbconnenction->execute( &queries);
}

void afsql::ResetStat( DBConnection * dbconnenction)
{
   if( dbconnenction->isWorking() == false ) return;
   if( dbconnenction->isOpen()    == false )
   {
      AFERROR("DBConnection::ResetArchive: Database connection is not open")
      return;
   }
   DBStatistics statistics;
   dbconnenction->dropTable( statistics.dbGetTableName());
   std::list<std::string> queries;
   statistics.dbCreateTable( &queries);
   dbconnenction->execute( &queries);
}

void afsql::ResetAll( DBConnection * dbconnenction)
{
   if( dbconnenction->isWorking() == false ) return;
   if( dbconnenction->isOpen()    == false )
   {
      AFERROR("DBConnection::ResetArchive: Database connection is not open")
      return;
   }
   dbconnenction->dropAllTables();
   ResetJobs(    dbconnenction);
   ResetUsers(   dbconnenction);
   ResetRenders( dbconnenction);
   ResetStat(    dbconnenction);
}

void afsql::UpdateTables( DBConnection * dbconnenction, bool showOnly )
{
   if( dbconnenction->isWorking() == false ) return;
   if( dbconnenction->isOpen()    == false )
   {
      AFERROR("DBConnection::Update: Database connection is not open")
      return;
   }

   std::list<std::string> columns;
   std::list<std::string> queries;

   DBTaskData taskdata;
   columns = dbconnenction->getTableColumnsNames( taskdata.dbGetTableName());
   taskdata.dbUpdateTable( &queries, columns);

   DBTaskProgress taskprogress;
   columns = dbconnenction->getTableColumnsNames( taskprogress.dbGetTableName());
   taskprogress.dbUpdateTable( &queries, columns);

   DBBlockData blockdata;
   columns = dbconnenction->getTableColumnsNames( blockdata.dbGetTableName());
   blockdata.dbUpdateTable( &queries, columns);

   DBJob job;
   columns = dbconnenction->getTableColumnsNames( job.dbGetTableName());
   job.dbUpdateTable( &queries, columns);

   DBUser user;
   columns = dbconnenction->getTableColumnsNames( user.dbGetTableName());
   user.dbUpdateTable( &queries, columns);

   DBRender render;
   columns = dbconnenction->getTableColumnsNames( render.dbGetTableName());
   render.dbUpdateTable( &queries, columns);

   DBStatistics statistics;
   columns = dbconnenction->getTableColumnsNames( statistics.dbGetTableName());
   statistics.dbUpdateTable( &queries, columns);

   if( false == showOnly ) dbconnenction->execute( &queries);
}
