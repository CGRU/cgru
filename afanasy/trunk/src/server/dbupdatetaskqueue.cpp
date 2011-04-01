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
}

DBTaskUpdateData::~DBTaskUpdateData(){}

DBUpdateTaskQueue::DBUpdateTaskQueue( const std::string & QueueName):
   AfQueue( QueueName)
{
   db = afsql::newDatabase( QueueName.c_str());
   db->open();
   query = new QSqlQuery( *db);
   query->prepare( afsql::DBTaskProgress::dbPrepareUpdate);
}

DBUpdateTaskQueue::~DBUpdateTaskQueue()
{
   delete query;
   db->close();
   delete db;
}

void DBUpdateTaskQueue::quit()
{
   db->close();
   AfQueue::quit();
}

void DBUpdateTaskQueue::processItem( AfQueueItem* item) const
{
//printf("DBUpdateTaskQueue::processItem:\n");
   if( false == db->open()) return;

   DBTaskUpdateData * taskUp = (DBTaskUpdateData*)item;

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

   query->exec();
   afsql::qChkErr( *query," UPDATE ...tasks:\n");
}
