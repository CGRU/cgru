#include "dbstatjob.h"

#include <job.h>

#include "dbattr.h"

using namespace afsql;

const QString DBStatJob::TableName("stat_jobs");

DBStatJob::DBStatJob()
{
   dbAddAttr( new DBAttrString( DBAttr::_name,              &name             ));
   dbAddAttr( new DBAttrString( DBAttr::_description,       &description      ));
   dbAddAttr( new DBAttrString( DBAttr::_username,          &username         ));
   dbAddAttr( new DBAttrString( DBAttr::_hostname,          &hostname         ));
   dbAddAttr( new DBAttrUInt32( DBAttr::_flags,             &flags            ));
   dbAddAttr( new DBAttrUInt32( DBAttr::_time_started,      &time_started     ));
   dbAddAttr( new DBAttrUInt32( DBAttr::_time_done,         &time_done        ));
   dbAddAttr( new DBAttrUInt32( DBAttr::_taskssumruntime,   &taskssumruntime  ));
}

DBStatJob::~DBStatJob()
{
}

void DBStatJob::dbDropTable( QSqlDatabase * db) const
{
   statblock.dbDropTable( db);
   DBItem::dbDropTable( db);
}

void DBStatJob::dbCreateTable( QSqlDatabase * db) const
{
   DBItem::dbCreateTable( db);
   statblock.dbCreateTable( db);
}

void DBStatJob::addJob( const af::Job * job, QStringList * queries)
{
   if( job->getBlocksNum() == 0) return;
   for( int b = 0; b < job->getBlocksNum(); b++) if( job->getBlock(b) == NULL) return;

   name = job->getName();
   flags = job->getFlags();
   description = job->getDescription();
   username = job->getUserName();
   hostname = job->getHostName();
   time_started = job->getTimeStarted();
   time_done = job->getTimeDone();
   taskssumruntime = 0;
   for( int b = 0; b < job->getBlocksNum(); b++)
   {
      taskssumruntime += job->getBlock(b)->getProgressTasksSumRunTime();
      statblock.addBlocks( job, queries);
   }
   dbInsert( queries);
}
