#include "dbactionqueue.h"

#include <QtCore/QVariant>
#include <QtSql/QSqlQuery>

#include "../include/afanasy.h"

#include "../libafsql/dbjob.h"
#include "../libafsql/dbrender.h"
#include "../libafsql/dbuser.h"

#include "afcommon.h"
#include "monitorcontainer.h"

extern bool running;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

DBActionQueue::DBActionQueue( const std::string & QueueName, MonitorContainer * monitorcontainer):
   AfQueue( QueueName),
   monitors( monitorcontainer)
{
   db = afsql::newDatabase( QueueName.c_str());
   working = db->open();
   if( false == working )
   {
      printf("Database connection \"%s\" is not working.\n", name.c_str());
   }
   else
   {
      connectionEstablished();
   }
}

DBActionQueue::~DBActionQueue()
{
   db->close();
   delete db;
   QSqlDatabase::removeDatabase( name.c_str());
}

void DBActionQueue::connectionEstablished()
{
   AFINFA("DBActionQueue::connectionEstablished: %s", name.c_str())
}

void DBActionQueue::processItem( AfQueueItem* item)
{
//printf("DBActionQueue::processItem: %s:\n", name.c_str());
   if( false == working )
   {
      delete item;
      return;
   }
   if( false == db->isOpen())
   {
      for(;;)
      {
         if( false == running )
         {
            delete item;
            return;
         }
         if( db )
         {
            afsql::setDatabase( db);
            db->open();
            if( db->isOpen())
            {
               connectionEstablished();
               sendConnected();
               break;
            }
         }
         sendAlarm();
         sleep( AFDATABASE::RECONNECTAFTER);
      }
   }

   // Writing an item and check if error:
   if( false == writeItem( item))
   {
      // Check if database has just closed:
      if( false == db->isOpen())
      {
         // Push item back to queue front to try it to write again next time:
         push( item, true );
         AFINFA("%s: Item pushed back to queue front.", name.c_str())
         return;
      }
   }
   delete item;
}

bool DBActionQueue::writeItem( AfQueueItem* item)
{
//printf("DBActionQueue::writeItem:\n");
   Queries * queries = (Queries*)item;

   int size = queries->size();
   if( size < 1) return true;

   QSqlQuery query( *db);
   for( std::list<std::string>::const_iterator it = queries->begin(); it != queries->end(); it++)
   {
      if( false == db->isOpen()) return false;
      AFINFA("DBActionQueue::writeItem: %s: Executing query:\n%s", name.c_str(), (*it).c_str())
      query.exec( afsql::stoq(*it));
      if( afsql::qChkErr( query, name)) return false;
   }
   return true;
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

void DBActionQueue::sendAlarm()
{
   std::string str("ALARM! Server database connection error. Contact your system administrator.");
   AFCommon::QueueLog( name + ":\n" + str);
   AfContainerLock mLock( monitors, AfContainerLock::WRITELOCK);
   monitors->sendMessage( str);
}

void DBActionQueue::sendConnected()
{
   std::string str("AFANASY: Server database connection established.");
   AFCommon::QueueLog( name + ":\n" + str);
   AfContainerLock mLock( monitors, AfContainerLock::WRITELOCK);
   monitors->sendMessage( str);
}
