#include "dbconnection.h"

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

bool DBConnection::ms_enabled = true;

DBConnection::DBConnection( const std::string & i_connection_name):
    m_name( i_connection_name),
    m_conn( NULL),
    m_working( false),
    m_opened( false)
{
    if( false == ms_enabled )
        return;

    m_conn = PQconnectdb( af::Environment::get_DB_ConnInfo().c_str());
    if( PQstatus( m_conn) != CONNECTION_OK)
    {
        AFERRAR("%s: Database connect failed: %s", m_name.c_str(), PQerrorMessage( m_conn));
    }
    else
    {
        m_working = true;
    }
    PQfinish( m_conn);
    m_conn = NULL;
}

DBConnection::~DBConnection()
{
    if( m_conn != NULL)
    {
        PQfinish( m_conn);
    }
}

bool DBConnection::DBOpen()
{
   if( m_working == false ) return false;
   AFINFO("Trying to lock DB...")

   m_mutex.Lock();

   AFINFO(" - Done")

   if( m_opened )
   {
      AFERRAR("%s: database is already open:", m_name.c_str())

        m_mutex.Unlock();

      return false;
   }

    m_conn = PQconnectdb( af::Environment::get_DB_ConnInfo().c_str());
    if( PQstatus( m_conn) != CONNECTION_OK)
    {
        AFERRAR("%s: Database connect failed: %s", m_name.c_str(), PQerrorMessage( m_conn));

        m_mutex.Unlock();

        PQfinish( m_conn);
        m_conn = NULL;

        return false;
    }

   m_opened = true;

   return true;
}

void DBConnection::DBClose()
{
    if( m_working == false )
    {
        return;
    }

    PQfinish( m_conn);
    m_conn = NULL;

    m_opened = false;

    m_mutex.Unlock();

    AFINFO("DB Unlocked.")
}

bool DBConnection::dropAllTables()
{
    if( m_working == false )
    {
        return false;
    }
    if( false == m_opened )
    {
        AFERRAR("DBConnection::dropAllTables: Database connection '%s' is not open.", m_name.c_str())
        return false;
    }

    char query[] = "select relname from pg_stat_user_tables WHERE schemaname='public';";
    PGresult * res = PQexec( m_conn, query);
    if( PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        AFERRAR("%s: Getting all tables names failed:\n%s\n%s",
                m_name.c_str(), query, PQerrorMessage( m_conn))
        return false;
    }

    std::list<std::string> tables;
    for( int i = 0; i < PQntuples( res); i++)
    {
        for( int j = 0; j < PQnfields( res); j++)
        {
            tables.push_back( PQgetvalue(res, i, j));
        }
    }
    PQclear( res);

    bool o_result = true;
    for( std::list<std::string>::const_iterator it = tables.begin(); it != tables.end(); it++)
    {
        std::string query = std::string("DROP TABLE ") + *it + " CASCADE;";
        std::cout << query << std::endl;
        PGresult * res = PQexec( m_conn, query.c_str());
        if( PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            AFERRAR("%s: Drop table failed: %s",
                    m_name.c_str(), PQerrorMessage( m_conn));
            o_result = false;
        }
        PQclear( res);
    }

    return o_result;
}

bool DBConnection::getItem( DBItem * item)
{
   if( m_working == false ) return false;
   return item->v_dbSelect( m_conn);
}

bool DBConnection::addJob( DBJob * job)
{
   if( m_working == false ) return false;
   return job->dbAdd( m_conn);
}

bool DBConnection::execute( const std::list<std::string> * i_queries)
{
    if( m_working == false )
    {
        return false;
    }
    AFINFA("%s: Executing queries:", m_name.c_str())

    return afsql::execute( m_conn, i_queries);
}

const std::list<int> DBConnection::getIntegers( const std::string & i_query)
{
    std::list<int> o_intlinst;
    if( m_working == false )
    {
        return o_intlinst;
    }
    if( m_opened == false )
    {
        AFERRAR("DBConnection::getIntegers: Database '%s' connection is not open.", m_name.c_str())
        return o_intlinst;
    }

    #ifdef AFOUTPUT
    printf("DBConnection::getIntegers: Executing query:\n");
    printf("%s\n", i_query.c_str());
    #endif

    PGresult * res = PQexec( m_conn, i_query.c_str());
    if( PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        AFERRAR("%s: Getting numbers failed:\n%s\n%s",
                m_name.c_str(), i_query.c_str(), PQerrorMessage( m_conn));
        return o_intlinst;
    }

    for( int i = 0; i < PQntuples( res); i++)
    {
        for( int j = 0; j < PQnfields( res); j++)
        {
            o_intlinst.push_back( af::stoi( PQgetvalue(res, i, j)));
        }
    }

    PQclear( res);

    #ifdef AFOUTPUT
    for( std::list<int>::iterator it = o_intlinst.begin(); it != o_intlinst.end(); it++)
        printf(" %d", *it);
    printf("\n");
    #endif

    return o_intlinst;
}

bool DBConnection::dropTable( const std::string & i_tablename)
{
    if( m_opened == false )
    {
        AFERRAR("DBConnection::dropTable: Database connection '%s is not open.", m_name.c_str())
        return false;
    }

    std::string query = std::string("DROP TABLE ") + i_tablename + ";";
    AFINFA("%s: executing query:\n%s", m_name.c_str(), query.c_str())
    PGresult * res = PQexec( m_conn, query.c_str());
    if( PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        AFERRAR("SQL Drop table failed: %s", PQerrorMessage( m_conn));
        return false;
    }

    PQclear( res);

    return true;
}

const std::list<std::string> DBConnection::getTableColumns( const std::string & i_tablename)
{
    std::list<std::string> o_columns;
    if( m_opened == false )
    {
        AFERRAR("DBConnection::getTableColumnsNames: Database '%s' connection is not open.", m_name.c_str())
        return o_columns;
    }
    std::string query = std::string("SELECT * FROM ") + i_tablename;

    PGresult * res = PQexec( m_conn, query.c_str());
    if( PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        AFERRAR("%s: Getting table colums failed:\n%s\n%s",
               m_name.c_str(), query.c_str(), PQerrorMessage( m_conn));
        return o_columns;
    }

    for( int i = 0; i < PQnfields( res); i++)
    {
        std::string fieldname = PQfname( res, i);
        if( fieldname.size())
        {
            o_columns.push_back( fieldname);
        }
    }

    #ifdef AFOUTPUT
    printf("DBConnection::getTableColumns: Table '%s' has columns:\n", i_tablename.c_str());
    for( std::list<std::string>::iterator it = o_columns.begin(); it != o_columns.end(); it++)
        printf(" %s", (*it).c_str());
    printf("\n");
    #endif

    PQclear( res);

    return o_columns;
}
