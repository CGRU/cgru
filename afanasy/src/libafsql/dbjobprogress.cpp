#include "dbjobprogress.h"

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

bool DBJobProgress::dbAdd( PGconn * i_conn) const
{
    AFINFO("DBJobProgress::dbAdd:")

    DBTaskProgress::dbPrepareInsert( i_conn);

    int id_job = getJobId();
    for( int b = 0; b < m_blocks_num; b++)
    {
        int id_block(b);
        for( int t = 0; t < tasksnum[b]; t++)
        {
            if( false == DBTaskProgress::dbPrepareInsertExec( id_job, id_block, t, i_conn))
            {
                AFERROR("Failed to add task progress in database.\n");
                return false;
            }
        }
    }

    return true;
}

bool DBJobProgress::dbSelect( PGconn * i_conn)
{
   for( int b = 0; b < m_blocks_num; b++)
   {
      for( int t = 0; t < tasksnum[b]; t++)
      {
         std::string where = DBTaskProgress::dbWhereSelect( getJobId(), b, t);
         if( ((DBTaskProgress*)(tp[b][t]))->v_dbSelect( i_conn, &where) == false) return false;
      }
   }
   return true;
}
