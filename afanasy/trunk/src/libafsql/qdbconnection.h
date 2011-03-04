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

class QCoreApplication;
class QSqlDatabase;

class afsql::DBConnection
{
public:
   DBConnection( const std::string & connection_name = std::string("AFLIBSQL"));
   ~DBConnection();

   void ResetJobs();
   void ResetStat();
   void ResetUsers();
   void ResetRenders();
   void ResetAll();

   void getUsersIds(   std::list<int32_t> & ids);
   void getRendersIds( std::list<int32_t> & ids);
   void getJobsIds(    std::list<int32_t> & ids);

   bool getItem( DBItem * item);

   void addJob( DBJob * job);

   inline bool isWorking() const { return working;}

   void execute( QStringList const & queries);

   bool DBOpen();
   void DBClose();

private:
   bool working;

#ifdef MACOSX
   QMutex        q_mutex;
#else
   pthread_mutex_t mutex;
#endif

   QSqlDatabase * db;
};
