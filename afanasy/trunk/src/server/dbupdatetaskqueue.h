#pragma once

#include "dbactionqueue.h"

class DBTaskUpdateData: public AfQueueItem
{
public:

   DBTaskUpdateData( int JobId, int BlockNum, int TaskNum, const af::TaskProgress * Progress);
   ~DBTaskUpdateData();

   int jobid;
   int blocknum;
   int tasknum;

   uint32_t state;
   uint32_t time_start;
   uint32_t time_done;
   int      starts_count;
   int      errors_count;

};

/// Simple FIFO update tast in database queue
class DBUpdateTaskQueue : public DBActionQueue
{
public:
   DBUpdateTaskQueue( const std::string & QueueName, MonitorContainer * monitorcontainer);
   virtual ~DBUpdateTaskQueue();

/// Push task update data to queue back.
   inline bool pushTaskUp(  int JobId, int BlockNum, int TaskNum, const af::TaskProgress * Progress)
      { if(isWorking()) return push( new DBTaskUpdateData( JobId, BlockNum, TaskNum, Progress)); else return false;}

protected:
   /// Called when database connection opened (or reopened)
   virtual void connectionEstablished();

   /// Queries execution function
   virtual bool writeItem( AfQueueItem* item);

   QSqlQuery * query;
};
