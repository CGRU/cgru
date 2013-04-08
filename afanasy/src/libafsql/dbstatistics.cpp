#include "dbstatistics.h"

#include "../libafanasy/job.h"

#include "dbattr.h"

using namespace afsql;

const std::string DBStatistics::TableName("statistics");

DBStatistics::DBStatistics()
{
   dbAddAttr( new DBAttrString(  DBAttr::_annotation,        &annotation       ));
   dbAddAttr( new DBAttrString(  DBAttr::_blockname,         &blockname        ));
   dbAddAttr( new DBAttrString(  DBAttr::_jobname,           &jobname          ));
   dbAddAttr( new DBAttrString(  DBAttr::_description,       &description      ));
   dbAddAttr( new DBAttrString(  DBAttr::_hostname,          &hostname         ));
   dbAddAttr( new DBAttrString(  DBAttr::_service,           &service          ));
   dbAddAttr( new DBAttrUInt32(  DBAttr::_tasksdone,         &tasksdone        ));
   dbAddAttr( new DBAttrUInt32(  DBAttr::_tasksnum,          &tasksnum         ));
   dbAddAttr( new DBAttrInt64 (  DBAttr::_taskssumruntime,   &taskssumruntime  ));
   dbAddAttr( new DBAttrInt64 (  DBAttr::_time_done,         &time_done        ));
   dbAddAttr( new DBAttrInt64 (  DBAttr::_time_started,      &time_started     ));
   dbAddAttr( new DBAttrString(  DBAttr::_username,          &username         ));
}

DBStatistics::~DBStatistics()
{
}

void DBStatistics::addJob( const af::Job * job, std::list<std::string> * queries)
{
   // Get job parameters:
   jobname        = job->getName();
   description    = job->getDescription();
   annotation     = job->getAnnotation();
   username       = job->getUserName();
   hostname       = job->getHostName();
   time_started   = job->getTimeStarted();
   time_done      = job->getTimeDone();

   // Skip not started job:
   if( time_started == 0 ) return;
   // Skip job with no running time:
   if( time_started == time_done ) return;
   // Set done time to current time, if job was not done:
   if( time_done < time_started ) time_done = time( NULL);

   // Inserting each block in table:
   for( int b = 0; b < job->getBlocksNum(); b++)
   {
      // Get block parameters:
      af::BlockData * block = job->getBlock(b);
      blockname = block->getName();
      service = block->getService();
      tasksnum = block->getTasksNum();
      tasksdone = block->getProgressTasksDone();
      taskssumruntime = job->getBlock(b)->getProgressTasksSumRunTime();

      // Skip blocks with no run time:
      if( tasksnum == 0 ) continue;
      if( tasksdone == 0 ) continue;
      if( taskssumruntime == 0 ) continue;

      // Insert row:
      v_dbInsert( queries);
   }
}
