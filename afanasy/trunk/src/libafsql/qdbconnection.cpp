#include "qdbconnection.h"

#include <stdio.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qvariant.h>
#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlerror.h>
#include <QtSql/qsqlquery.h>

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
   working( false)
{
   db = newDatabase( connection_name);

#ifndef MACOSX
   if( pthread_mutex_init( &mutex, NULL) != 0)
   {
      AFERRPE("DBConnection::DBConnection: pthread_mutex_init:")
      return;
   }
#endif

   working = true;
   working = DBOpen();
   DBClose();
}

DBConnection::~DBConnection()
{
   db->close();

#ifndef MACOSX
   if( pthread_mutex_destroy( &mutex) != 0)
      AFERRPE("DBConnection::DBConnection: pthread_mutex_destroy:")
#endif

   delete db;
}

bool DBConnection::DBOpen()
{
   if( working == false ) return false;
#ifdef _DEBUG
printf("Trying to lock DB...\n");
#endif

#ifdef MACOSX
   q_mutex.lock();
#else
  if( pthread_mutex_lock( &mutex) != 0)
   {
      AFERRPE("DBConnection::DBOpen: pthread_mutex_unlock:")
      return false;
   }
#endif

#ifdef _DEBUG
printf(" - Done\n");
#endif
   if( db->isOpen())
   {
      AFERROR("DBConnection::DBOpen: database is already open:")
#ifdef MACOSX
   q_mutex.unlock();
#else
      if( pthread_mutex_unlock( &mutex) != 0)
         AFERRPE("DBConnection::DBOpen: pthread_mutex_unlock:");
#endif
      return false;
   }

   if( db->open() == false )
   {
      AFERROR("DBConnection::DBOpen: UNABLE TO OPEN DATABASE:")
      printf( "%s\n", db->lastError().text().toUtf8().data());

#ifdef MACOSX
      q_mutex.unlock();
#else
      if( pthread_mutex_unlock( &mutex) != 0)
         AFERRPE("DBConnection::DBOpen: pthread_mutex_unlock:");
#endif

      return false;
   }
   else return true;
}

void DBConnection::DBClose()
{
   db->close();

#ifdef MACOSX
      q_mutex.unlock();
#else
   if( pthread_mutex_unlock( &mutex) != 0)
      AFERRPE("DBConnection::close: pthread_mutex_unlock:");
#endif

#ifdef _DEBUG
printf("DB Unlocked.");
#endif
}

void DBConnection::ResetUsers()
{
   if( working == false ) return;
   if( db->isOpen() == false )
   {
      AFERROR("DBConnection::ResetUsers: Database connection is not open")
      return;
   }

   DBUser user;
   user.dbDropTable( db);
   user.dbCreateTable( db);
}

void DBConnection::ResetRenders()
{
   if( working == false ) return;
   if( db->isOpen() == false )
   {
      AFERROR("DBConnection::ResetRenders: Database connection is not open")
      return;
   }

   DBRender render;
   render.dbDropTable( db);
   render.dbCreateTable( db);
}

void DBConnection::ResetJobs()
{
   if( working == false ) return;
   if( db->isOpen() == false )
   {
      AFERROR("DBConnection::ResetOnline: Database connection is not open")
      return;
   }

   DBJob job;
   DBBlockData block;
   DBTaskData task;
   DBTaskProgress progress;

   progress.dbDropTable( db);
   task.dbDropTable( db);
   block.dbDropTable( db);
   job.dbDropTable( db);

   job.dbCreateTable( db);
   block.dbCreateTable( db);
   task.dbCreateTable( db);
   progress.dbCreateTable( db);
}

void DBConnection::ResetStat()
{
   if( working == false ) return;
   if( db->isOpen() == false )
   {
      AFERROR("DBConnection::ResetArchive: Database connection is not open")
      return;
   }

   DBStatistics statistics;
   statistics.dbDropTable( db);
   statistics.dbCreateTable( db);
}

void DBConnection::ResetAll()
{
   QStringList tables = db->tables();
   for( int i = 0; i < tables.size(); i++)
   {
      QString query = QString("DROP TABLE %1;").arg( tables[i]);
      printf("%s\n", query.toUtf8().data());
      QSqlQuery q( query, *db);
      q.exec();
      qChkErr(q, "DBConnection::ResetAll:\n");
   }

   ResetStat();
   ResetUsers();
   ResetRenders();
   ResetJobs();
}

void DBConnection::getUsersIds( std::list<int32_t> & ids)
{
   if( working == false ) return;
   DBUser::getIds( ids, db);
}
void DBConnection::getRendersIds( std::list<int32_t> & ids)
{
   if( working == false ) return;
   DBRender::getIds( ids, db);
}
void DBConnection::getJobsIds( std::list<int32_t> & ids)
{
   if( working == false ) return;
   DBJob::getIds( ids, db);
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

void DBConnection::execute( QStringList const & queries)
{
   if( working == false ) return;
   AFINFO("Executing queries:")
   for( int i = 0; i < queries.size(); i++)
   {
      QSqlQuery q( queries[i], *db);
      q.exec();
#ifdef AFOUTPUT
printf("%s\n", queries[i].toUtf8().data());
#endif
   }
}
