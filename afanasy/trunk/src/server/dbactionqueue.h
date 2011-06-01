#pragma once

#include <QtSql/QSqlDatabase>
#include <QtCore/QStringList>

#include "../libafanasy/name_af.h"

#include "../libafsql/name_afsql.h"

#include "afqueue.h"

class MonitorContainer;

class Queries: public std::list<std::string>, public AfQueueItem {};

/// Simple FIFO database action queue
class DBActionQueue : public AfQueue
{
public:
   DBActionQueue( const std::string & QueueName, MonitorContainer * monitorcontainer);
   virtual ~DBActionQueue();

   inline bool isWorking() const { return working;}

   void addItem(    const afsql::DBItem * item);
   void delItem(    const afsql::DBItem * item);
   void updateItem( const afsql::DBItem * item, int attr = -1);

   void quit();

/// Push queries to queue back.
   inline bool pushQueries( Queries * queries) { if( working ) return push( queries); else return false;}

protected:

   /// Called from run thead to process item just poped from queue
   virtual void processItem( AfQueueItem* item);

   /// Called when database connection opened (or reopened)
   virtual void connectionEstablished();

   /// Queries execution function
   virtual bool writeItem(   AfQueueItem* item);

   QSqlDatabase * db;

private:
   void sendAlarm();
   void sendConnected();

private:
   MonitorContainer * monitors;
   bool working;
};
