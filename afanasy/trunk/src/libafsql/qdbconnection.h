#pragma once

#ifdef MACOSX
#include <QtCore/QMutex>
#else
#include <pthread.h>
#endif

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "../libafanasy/name_af.h"

#include "name_afsql.h"

class QSqlDatabase;

class afsql::DBConnection
{
public:
   DBConnection( const std::string & connection_name = std::string("AFLIBSQL"));
   ~DBConnection();

   inline bool isWorking() const { return working;       }
   inline bool isOpen()    const { return db->isOpen();  }

   void execute( const std::list<std::string> * queries);

   const std::list<int> getIntegers( const std::string & query);

   bool getItem( DBItem * item);

   void addJob( DBJob * job);

   bool DBOpen();
   void DBClose();

   const std::list<std::string> getTableColumnsNames( const std::string & tableName);
   void dropTable( const std::string & tableName);
   void dropAllTables();

private:
   std::string name;
   bool working;

#ifdef MACOSX
   QMutex        q_mutex;
#else
   pthread_mutex_t mutex;
#endif

   QSqlDatabase * db;
};
