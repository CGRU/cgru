#include "dbupdatetaskqueue.h"

#include <QtCore/QVariant>

#include "../libafanasy/jobprogress.h"

#include "../libafsql/name_afsql.h"
#include "../libafsql/dbtaskprogress.h"

#include "afcommon.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

DBTaskUpdateData::DBTaskUpdateData( int JobId, int BlockNum, int TaskNum, const af::TaskProgress * Progress):
   jobid(         JobId                   ),
   blocknum(      BlockNum                ),
   tasknum(       TaskNum                 ),
   state(         Progress->state         ),
   time_start(    Progress->time_start    ),
   time_done(     Progress->time_done     ),
   starts_count(  Progress->starts_count  ),
   errors_count(  Progress->errors_count  )
{
AFINFA("DBTaskUpdateData::DBTaskUpdateData: %d[%d][%d]", jobid, blocknum, tasknum)
}

DBTaskUpdateData::~DBTaskUpdateData(){}

DBUpdateTaskQueue::DBUpdateTaskQueue( const std::string & QueueName, MonitorContainer * monitorcontainer):
   DBActionQueue( QueueName, monitorcontainer),
   query( NULL)
{
   if( isWorking()) connectionEstablished();
}

DBUpdateTaskQueue::~DBUpdateTaskQueue()
{
   delete query;
}

void DBUpdateTaskQueue::connectionEstablished()
{
AFINFA("DBUpdateTaskQueue::connectionEstablished: %s", name.c_str())

   if( query ) delete query;
   query = new QSqlQuery( *db);
   query->prepare( afsql::stoq(afsql::DBTaskProgress::dbPrepareUpdate));
}

bool DBUpdateTaskQueue::writeItem( AfQueueItem* item)
{
   DBTaskUpdateData * taskUp = (DBTaskUpdateData*)item;

AFINFA("DBUpdateTaskQueue::writeItem: %d[%d][%d]", taskUp->jobid, taskUp->blocknum, taskUp->tasknum)

   afsql::DBTaskProgress::dbBindUpdate
   (
      query,
      taskUp->state,
      taskUp->starts_count,
      taskUp->errors_count,
      taskUp->time_start,
      taskUp->time_done,
      taskUp->jobid,
      taskUp->blocknum,
      taskUp->tasknum
   );

   if( false == db->isOpen()) return false;
   query->exec();
   if( afsql::qChkErr( *query, name)) return false;
   return true;
}
