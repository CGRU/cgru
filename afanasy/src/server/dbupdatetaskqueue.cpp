#include "dbupdatetaskqueue.h"

#include "../libafanasy/jobprogress.h"

#include "../libafsql/name_afsql.h"
#include "../libafsql/dbtaskprogress.h"

#include "afcommon.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

char DBTaskUpdateData::ms_db_prepare_name[] = "tasks_progress_update";

DBTaskUpdateData::DBTaskUpdateData( int i_id_job, int i_id_block, int i_id_task, const af::TaskProgress * i_taskprogress):
    m_id_job(        i_id_job                      ),
    m_id_block(      i_id_block                    ),
    m_id_task(       i_id_task                     ),
    m_state(         i_taskprogress->state         ),
    m_time_start(    i_taskprogress->time_start    ),
    m_time_done(     i_taskprogress->time_done     ),
    m_starts_count(  i_taskprogress->starts_count  ),
    m_errors_count(  i_taskprogress->errors_count  )
{
    AFINFA( "DBTaskUpdateData::DBTaskUpdateData: %d[%d][%d]", i_id_job, i_id_block, i_id_task )
}

DBTaskUpdateData::~DBTaskUpdateData(){}

bool DBTaskUpdateData::prepare( PGconn * i_conn)
{
//  FIXME: How does binary interface work? Were to find some help/exmples?
/* To get Oid execute query: SELECT oid,typname from pg_type; */
//    static const Oid types[] = {20,20,20,20,23,23,20,20};

    PGresult * res = PQprepare( i_conn, ms_db_prepare_name,
                                "UPDATE progress SET"
                                " state = $1,"
                                " starts_count = $2,"
                                " errors_count = $3,"
                                " time_started = $4,"
                                " time_done = $5"
                                " WHERE id_job = $6"
                                " AND id_block = $7"
                                " AND id_task = $8;"
    ,8 , NULL /*, types*/);
    bool o_result = true;
    if( PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        AFERRAR("SQL Preparing tasks data progress update failed: %s", PQerrorMessage( i_conn));
        o_result = false;
    }
    PQclear( res);
    return o_result;
}

bool DBTaskUpdateData::prepareExec( PGconn * i_conn) const
{
    const char * data_pointers[8];

    std::string id_job_str(   af::itos( m_id_job    ));
    std::string id_block_str( af::itos( m_id_block  ));
    std::string id_task_str(  af::itos( m_id_task   ));

    std::string state_str(        af::itos( m_state        ));
    std::string starts_count_str( af::itos( m_starts_count ));
    std::string errors_count_str( af::itos( m_errors_count ));
    std::string time_start_str(   af::itos( m_time_start   ));
    std::string time_done_str(    af::itos( m_time_done    ));

    data_pointers[0] = state_str.c_str();
    data_pointers[1] = starts_count_str.c_str();
    data_pointers[2] = errors_count_str.c_str();
    data_pointers[3] = time_start_str.c_str();
    data_pointers[4] = time_done_str.c_str();

    data_pointers[5] = id_job_str.c_str();
    data_pointers[6] = id_block_str.c_str();
    data_pointers[7] = id_task_str.c_str();
/*
//  FIXME: How does binary interface work? Were to find some help/exmples?
    data_pointers[0] = (char*)m_state;
    data_pointers[1] = (char*)m_starts_count;
    data_pointers[2] = (char*)m_errors_count;
    data_pointers[3] = (char*)m_time_start;
    data_pointers[4] = (char*)m_time_done;

    data_pointers[5] = (char*)m_id_job;
    data_pointers[6] = (char*)m_id_block;
    data_pointers[7] = (char*)m_id_task;

    int paramLengths[] = {4,4,4,8,8,4,4,4};
    int paramFormats[] = {1,1,1,1,1,1,1,1};
*/
    PGresult * res = PQexecPrepared( i_conn, ms_db_prepare_name,
                                     8, data_pointers,
                                     NULL, NULL, 0);
//                                     paramLengths, paramFormats, 0);
    bool o_result = true;
    if( PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        AFERRAR("SQL Executing prepared tasks progress update failed: %s", PQerrorMessage( i_conn));
        o_result = false;
    }
    PQclear( res);
    return o_result;
}

DBUpdateTaskQueue::DBUpdateTaskQueue( const std::string & i_name, MonitorContainer * i_monitorcontainer):
    DBActionQueue( i_name, i_monitorcontainer),
    m_db_prepared( false)
{
    if( isWorking()) connectionEstablished();
}

DBUpdateTaskQueue::~DBUpdateTaskQueue()
{
}

void DBUpdateTaskQueue::connectionEstablished()
{
    AFINFA("DBUpdateTaskQueue::connectionEstablished: %s", name.c_str())

    m_db_prepared = DBTaskUpdateData::prepare( m_conn);
}

bool DBUpdateTaskQueue::writeItem( af::AfQueueItem* item)
{
    if( false == m_db_prepared )
    {
        // It should not happen
        return true;
    }

    DBTaskUpdateData * taskUp = (DBTaskUpdateData*)item;

    //AFINFA("DBUpdateTaskQueue::writeItem: %d[%d][%d]", taskUp->jobid, taskUp->blocknum, taskUp->tasknum)
    AFINFA( "DBUpdateTaskQueue::writeItem: %d[%d][%d]", taskUp->get_job_id(), taskUp->get_block_id(), taskUp->get_task_id() )

    if( PQstatus( m_conn) != CONNECTION_OK ) return false;

    return taskUp->prepareExec( m_conn );
}
