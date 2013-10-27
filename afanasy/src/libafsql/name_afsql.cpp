#include "name_afsql.h"

#include <stdio.h>

#include "../libafanasy/environment.h"

#include "dbattr.h"
#include "dbconnection.h"
#include "dbstatistics.h"

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
		AFERROR("No queries to execute.");
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

void afsql::ResetStat( DBConnection * dbconnenction)
{
	if( dbconnenction->isWorking() == false ) return;
	if( dbconnenction->isOpen()	 == false )
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
	if( dbconnenction->isOpen()	 == false )
	{
		AFERROR("DBConnection::ResetArchive: Database connection is not open.")
		return;
	}
	dbconnenction->dropAllTables();
	ResetStat( dbconnenction);
}

void afsql::UpdateTables( DBConnection * dbconnenction, bool showOnly )
{
	if( dbconnenction->isWorking() == false ) return;
	if( dbconnenction->isOpen()	 == false )
	{
		AFERROR("DBConnection::Update: Database connection is not open.")
		return;
	}

	std::list<std::string> columns;
	std::list<std::string> queries;

	DBStatistics statistics;
	columns = dbconnenction->getTableColumns( statistics.v_dbGetTableName());
	statistics.dbUpdateTable( &queries, columns);

	if(( queries.size()) && ( false == showOnly )) dbconnenction->execute( &queries);
}
