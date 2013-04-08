#include "dbtaskprogress.h"

#include "dbattr.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afsql;

const std::string DBTaskProgress::TableName("progress");
const std::string DBTaskProgress::IDs("id_job int, id_block int, id_task int");
const std::string DBTaskProgress::Keys(
"FOREIGN KEY ( id_job, id_block)"
" REFERENCES blocks ( id_job, id_block) ON DELETE CASCADE,"
" PRIMARY KEY ( id_job, id_block, id_task)");
char DBTaskProgress::ms_db_prepare_name[] = "task_progress_insert";

DBTaskProgress::DBTaskProgress():
   af::TaskProgress()
{
    dbAddAttr( new DBAttrUInt32( DBAttr::_state,              &state        ));
    dbAddAttr( new DBAttrInt32(  DBAttr::_starts_count,       &starts_count ));
    dbAddAttr( new DBAttrInt32(  DBAttr::_errors_count,       &errors_count ));
    dbAddAttr( new DBAttrInt64(  DBAttr::_time_started,       &time_start   ));
    dbAddAttr( new DBAttrInt64(  DBAttr::_time_done,          &time_done    ));
}

DBTaskProgress::~DBTaskProgress()
{
    AFINFO("DBTaskProgress::~DBTaskProgress")
}

bool DBTaskProgress::dbPrepareInsert( PGconn * i_conn)
{
    PGresult * res = PQprepare( i_conn, ms_db_prepare_name,
                                "INSERT INTO progress (id_job,id_block,id_task)"
                                " VALUES($1,$2,$3);"
                                , 3, NULL);
    bool o_result = true;
    if( PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        AFERRAR("SQL Preparing tasks progress insertion failed: %s", PQerrorMessage( i_conn));
        o_result = false;
    }
    PQclear( res);
    return o_result;
}

bool DBTaskProgress::dbPrepareInsertExec(  int i_id_job, int i_id_block, int i_id_task, PGconn * i_conn)
{
    std::string id_job_str(   af::itos( i_id_job   ));
    std::string id_block_str( af::itos( i_id_block ));
    std::string id_task_str(  af::itos( i_id_task  ));

    const char * data_pointers[3];

    data_pointers[0] = id_job_str.c_str();
    data_pointers[1] = id_block_str.c_str();
    data_pointers[2] = id_task_str.c_str();

    PGresult * res = PQexecPrepared( i_conn, ms_db_prepare_name,
                                     3, data_pointers,
                                     NULL, NULL, 0);
    bool o_result = true;
    if( PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        AFERRAR("SQL Executing prepared tasks progress insert failed: %s", PQerrorMessage( i_conn));
        o_result = false;
    }
    PQclear( res);
    return o_result;
}

int DBTaskProgress::v_calcWeight() const
{
    int o_weight = sizeof(DBTaskProgress) - sizeof(TaskProgress);
    o_weight += DBItem::v_calcWeight();
    return o_weight;
}
