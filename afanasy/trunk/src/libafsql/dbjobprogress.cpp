#include "dbjobprogress.h"

#include <QtCore/qvariant.h>

#include "../libafanasy/job.h"

#include "dbtaskprogress.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afsql;

DBJobProgress::DBJobProgress( af::Job * job):
   af::JobProgress( job, false)
{
   // Construct job in this parent class to call virtual functions to create custom classes
   construct( job);
/*   blocksnum = job->getBlocksNum();
   if( blocksnum < 1)
   {
      AFERRAR("DBJobProgress::JobProgress: invalid number if blocks = %d (jobid=%d)", blocksnum, job->getId())
      return;
   }

   if( initBlocks() == false)
   {
      AFERROR("DBJobProgress::JobProgress: blocks initalization failed.")
      return;
   }

   for( int b = 0; b < blocksnum; b++)
   {
      const af::BlockData * block = job->getBlock( b);
      tasksnum[b] = block->getTasksNum();
      if( tasksnum[b] < 1)
      {
         AFERRAR("DBJobProgress::JobProgress: invalud number of tasks = %d (jobid=%d,block=%d)", tasksnum[b], job->getId(), b)
         continue;
      }

      if( initTasks( b, tasksnum[b]) == false)
      {
         AFERRAR("DBJobProgress::JobProgress: tasks initalization failed ( block=%d, tasks number=%d).", b, tasksnum[b])
         continue;
      }
   }*/
}

DBJobProgress::~DBJobProgress()
{
AFINFO("DBJobProgress::~DBJobProgress:")
}

af::TaskProgress * DBJobProgress::newTaskProgress() const
{
AFINFO("DBJobProgress::newTaskProgress:")
   return new DBTaskProgress;
}

void DBJobProgress::dbAdd( QSqlDatabase * db) const
{
AFINFO("DBJobProgress::dbAdd:")
   QSqlQuery q( *db);
   q.prepare( DBTaskProgress::dbPrepareInsert);

   QVariant job( getJobId());
   for( int b = 0; b < blocksnum; b++)
   {
      QVariant block(b);
      for( int t = 0; t < tasksnum[b]; t++)
      {
         DBTaskProgress::dbBindInsert( &q, job, block, QVariant(t));
         q.exec();
         if( qChkErr(q, "DBJobProgress::dbAdd:\n")) break;
      }
   }
}

bool DBJobProgress::dbSelect( QSqlDatabase * db)
{
   for( int b = 0; b < blocksnum; b++)
   {
      for( int t = 0; t < tasksnum[b]; t++)
      {
         QString where = DBTaskProgress::dbWhereSelect( getJobId(), b, t);
         if( ((DBTaskProgress*)(tp[b][t]))->dbSelect( db, &where) == false) return false;
      }
   }
   return true;
}
