#pragma once

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDatabase>

#include "../libafanasy/name_af.h"

#include "afqueue.h"
#include "afqueueitem.h"

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
class DBUpdateTaskQueue : public AfQueue
{
public:
   DBUpdateTaskQueue( const QString & QueueName);
   virtual ~DBUpdateTaskQueue();

/// Push task update data to queue back.
   inline bool pushTaskUp(  int JobId, int BlockNum, int TaskNum, const af::TaskProgress * Progress)
      { if(db->isOpen()) return push( new DBTaskUpdateData( JobId, BlockNum, TaskNum, Progress)); else return false;}

   void quit();

protected:
   void processItem( AfQueueItem* item) const;

   QSqlDatabase * db;
   QSqlQuery * query;
};
