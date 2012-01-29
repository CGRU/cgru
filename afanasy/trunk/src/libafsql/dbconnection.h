#pragma once

#include <libpq-fe.h>

#include "../libafanasy/dlMutex.h"
#include "../libafanasy/name_af.h"

#include "name_afsql.h"

class afsql::DBConnection
{
public:
   DBConnection( const std::string & i_connection_name = std::string("AFLIBSQL"));
   ~DBConnection();

   inline bool isWorking() const { return m_working;}
   inline bool isOpen()    const { return m_opened; }

   bool execute( const std::list<std::string> * i_queries);

   const std::list<int> getIntegers( const std::string & i_query);

   bool getItem( DBItem * i_item);

   bool addJob( DBJob * i_job);

   bool DBOpen();
   void DBClose();

   const std::list<std::string> getTableColumns( const std::string & i_tablename);
   bool dropTable( const std::string & i_tablename);
   bool dropAllTables();

private:
   std::string m_name;
   bool m_working;
   bool m_opened;

   DlMutex m_mutex;

   PGconn * m_conn;
};
