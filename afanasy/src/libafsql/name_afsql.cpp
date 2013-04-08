#include "name_afsql.h"

#include <stdio.h>

#include "../libafanasy/environment.h"
#include "../libafanasy/job.h"
#include "../libafanasy/blockdata.h"
#include "../libafanasy/user.h"
#include "../libafanasy/render.h"

#include "dbattr.h"
#include "dbblockdata.h"
#include "dbconnection.h"
#include "dbjob.h"
#include "dbrender.h"
#include "dbstatistics.h"
#include "dbtaskdata.h"
#include "dbtaskprogress.h"
#include "dbuser.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

void afsql::init()
{
    std::string nosql_flag("-nosql");
    af::Environment::addUsage( nosql_flag, "Disable SQL database connection.");
    if( af::Environment::hasArgument(nosql_flag))
    {
        afsql::DBConnection::disable();
        printf("SQL database connection disabled.\n");
    }

    DBAttr::init();
}

bool afsql::execute( PGconn * i_conn, const std::list<std::string> * i_queries)
{
    if( i_queries->size() < 1 )
    {
        AFERROR("No queries to execute.\n");
        return false;
    }

    bool o_result = true;
    for( std::list<std::string>::const_iterator it = i_queries->begin(); it != i_queries->end(); it++)
    {
        #ifdef AFOUTPUT
        printf("%s\n", (*it).c_str());
        #endif
        PGresult * res = PQexec( i_conn, (*it).c_str());
        if( PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            AFERRAR("SQL command execution faled:\n%s\n%s", (*it).c_str(), PQerrorMessage( i_conn));
            o_result = false;
        }
        PQclear( res);
    }
    return o_result;
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
   dbconnenction->dropTable( user.v_dbGetTableName());
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
   dbconnenction->dropTable( render.v_dbGetTableName());
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

   dbconnenction->dropTable( progress.v_dbGetTableName());
   dbconnenction->dropTable( task.v_dbGetTableName());
   dbconnenction->dropTable( block.v_dbGetTableName());
   dbconnenction->dropTable( job.v_dbGetTableName());

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
   dbconnenction->dropTable( statistics.v_dbGetTableName());
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
   columns = dbconnenction->getTableColumns( taskdata.v_dbGetTableName());
   taskdata.dbUpdateTable( &queries, columns);

   DBTaskProgress taskprogress;
   columns = dbconnenction->getTableColumns( taskprogress.v_dbGetTableName());
   taskprogress.dbUpdateTable( &queries, columns);

   DBBlockData blockdata;
   columns = dbconnenction->getTableColumns( blockdata.v_dbGetTableName());
   blockdata.dbUpdateTable( &queries, columns);

   DBJob job;
   columns = dbconnenction->getTableColumns( job.v_dbGetTableName());
   job.dbUpdateTable( &queries, columns);

   DBUser user;
   columns = dbconnenction->getTableColumns( user.v_dbGetTableName());
   user.dbUpdateTable( &queries, columns);

   DBRender render;
   columns = dbconnenction->getTableColumns( render.v_dbGetTableName());
   render.dbUpdateTable( &queries, columns);

   DBStatistics statistics;
   columns = dbconnenction->getTableColumns( statistics.v_dbGetTableName());
   statistics.dbUpdateTable( &queries, columns);

   if(( queries.size()) && ( false == showOnly )) dbconnenction->execute( &queries);
}
