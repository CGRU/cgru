#include "dbactionqueue.h"

#include <QtCore/QVariant>
#include <QtSql/QSqlQuery>

#include "../libafsql/dbjob.h"
#include "../libafsql/dbrender.h"
#include "../libafsql/dbuser.h"

#include "afcommon.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

DBActionQueue::DBActionQueue( const std::string & QueueName):
   AfQueue( QueueName)
{
   db = afsql::newDatabase( QueueName.c_str());
   db->open();
}

DBActionQueue::~DBActionQueue()
{
   db->close();
   delete db;
}

void DBActionQueue::processItem( AfQueueItem* item) const
{
   if( false == db->isOpen()) return;

   Queries * queries = (Queries*)item;

   int size = queries->size();
   if( size < 1) return;

   QSqlQuery query( *db);
   for( int i  = 0; i < size; i++)
   {
      query.exec( (*queries)[i]);
      afsql::qChkErr( query," DBActionQueue:\n");
   }
}

void DBActionQueue::quit()
{
   db->close();
   AfQueue::quit();
}

void DBActionQueue::addItem( const afsql::DBItem * item)
{
   Queries * queries = new Queries();
   item->dbInsert( queries);
   push( queries);
}

void DBActionQueue::updateItem( const afsql::DBItem * item, int attr)
{
   Queries * queries = new Queries();
   item->dbUpdate( queries, attr);
   push( queries);
}

void DBActionQueue::delItem( const afsql::DBItem * item)
{
   Queries * queries = new Queries();
   item->dbDelete( queries);
   push( queries);
}
