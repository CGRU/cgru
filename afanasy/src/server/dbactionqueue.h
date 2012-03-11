#pragma once

#include "../libafanasy/afqueue.h"

#include "../libafsql/name_afsql.h"

class MonitorContainer;

class Queries: public std::list<std::string>, public af::AfQueueItem {};

/// Simple FIFO database action queue
class DBActionQueue : public af::AfQueue
{
public:
   DBActionQueue( const std::string & i_name, MonitorContainer * i_monitorcontainer);
   virtual ~DBActionQueue();

   inline bool isWorking() const { return m_working;}

   void addItem(    const afsql::DBItem * item);
   void delItem(    const afsql::DBItem * item);
   void updateItem( const afsql::DBItem * item, int attr = -1);

protected:

   /// Called from run thead to process item just poped from queue
   virtual void processItem( af::AfQueueItem* item);

   /// Called when database connection opened (or reopened)
   virtual void connectionEstablished();

   /// Queries execution function
   virtual bool writeItem(   af::AfQueueItem* item);

   PGconn * m_conn;

private:
   void sendAlarm();
   void sendConnected();

private:
   MonitorContainer * m_monitors;
   bool m_working;
};
