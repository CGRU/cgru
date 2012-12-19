#pragma once

#include "dbactionqueue.h"

class DBTaskUpdateData: public af::AfQueueItem
{
public:

   DBTaskUpdateData( int i_id_job, int i_id_block, int i_id_task, const af::TaskProgress * i_taskprogress);
   ~DBTaskUpdateData();

   static bool prepare( PGconn * i_conn);
   bool prepareExec( PGconn * i_conn) const;
   
   int get_job_id() {return m_id_job; };
   int get_block_id() { return m_id_block; };
   int get_task_id() { return m_id_task; };

private:
    int m_id_job;
    int m_id_block;
    int m_id_task;

    uint32_t m_state;
    int64_t  m_time_start;
    int64_t  m_time_done;
    int32_t  m_starts_count;
    int32_t  m_errors_count;

    static char ms_db_prepare_name[];
};

/// Simple FIFO update tast in database queue
class DBUpdateTaskQueue : public DBActionQueue
{
public:
   DBUpdateTaskQueue( const std::string & i_name, MonitorContainer * i_monitorcontainer);
   virtual ~DBUpdateTaskQueue();

/// Push task update data to queue back.
   inline bool pushTaskUp(  int JobId, int BlockNum, int TaskNum, const af::TaskProgress * Progress)
      { if(isWorking()) return push( new DBTaskUpdateData( JobId, BlockNum, TaskNum, Progress)); else return false;}

protected:
   /// Called when database connection opened (or reopened)
   virtual void connectionEstablished();

   /// Queries execution function
   virtual bool writeItem( af::AfQueueItem* item);

   bool m_db_prepared;
};
