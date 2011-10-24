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
   q.prepare( afsql::stoq( DBTaskProgress::dbPrepareInsert));

   int id_job = getJobId();
   for( int b = 0; b < blocksnum; b++)
   {
      int id_block(b);
      for( int t = 0; t < tasksnum[b]; t++)
      {
         DBTaskProgress::dbBindInsert( &q, id_job, id_block, t);
         q.exec();
         if( qChkErr(q, "DBJobProgress::dbAdd:")) break;
      }
   }
}

bool DBJobProgress::dbSelect( QSqlDatabase * db)
{
   for( int b = 0; b < blocksnum; b++)
   {
      for( int t = 0; t < tasksnum[b]; t++)
      {
         std::string where = DBTaskProgress::dbWhereSelect( getJobId(), b, t);
         if( ((DBTaskProgress*)(tp[b][t]))->dbSelect( db, &where) == false) return false;
      }
   }
   return true;
}
