#include "dbstatblock.h"

#include "../libafanasy/job.h"

#include "dbattr.h"

using namespace afsql;

const QString DBStatBlock::TableName("stat_blocks");

DBStatBlock::DBStatBlock()
{
   dbAddAttr( new DBAttrString( DBAttr::_name,              &name             ));
   dbAddAttr( new DBAttrString( DBAttr::_name_job,          &name_job         ));
   dbAddAttr( new DBAttrUInt32( DBAttr::_flags,             &flags            ));
   dbAddAttr( new DBAttrString( DBAttr::_username,          &username         ));
   dbAddAttr( new DBAttrString( DBAttr::_taskstype,         &taskstype        ));
   dbAddAttr( new DBAttrUInt32( DBAttr::_tasksnum,          &tasksnum         ));
   dbAddAttr( new DBAttrUInt32( DBAttr::_tasksdone,         &tasksdone        ));
   dbAddAttr( new DBAttrUInt32( DBAttr::_taskssumruntime,   &taskssumruntime  ));
}

DBStatBlock::~DBStatBlock()
{
}

void DBStatBlock::addBlocks( const af::Job * job, QStringList * queries)
{
   name_job = job->getName();
   username = job->getUserName();

   for( int b = 0; b < job->getBlocksNum(); b++)
   {
      const af::BlockData *block = job->getBlock(b);
      if( block->getProgressTasksDone() < 1) continue;

      name = block->getName();
      flags = block->getFlags();
      taskstype = block->getTasksType();
      tasksnum = block->getTasksNum();
      tasksdone = block->getProgressTasksDone();
      taskssumruntime = block->getProgressTasksSumRunTime();

      dbInsert( queries);
   }
}
