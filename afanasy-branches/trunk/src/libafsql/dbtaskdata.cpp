#include "dbtaskdata.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "dbattr.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afsql;

const std::string DBTaskData::TableName("tasks");
const std::string DBTaskData::IDs("id_job int, id_block int, id_task int");
const std::string DBTaskData::Keys("FOREIGN KEY (id_job, id_block) REFERENCES blocks (id_job, id_block) ON DELETE CASCADE, PRIMARY KEY (id_job, id_block, id_task)");
/*
const std::string DBTaskData::dbPrepareInsert
("INSERT INTO tasks (id_job,id_block,id_task,name,command,files,dependmask,customdata)\
 VALUES(:id_job,:id_block,:id_task,:name,:command,:files,:dependmask,:customdata);\
");
*/
const char DBTaskData::ms_db_prepare_name[] = "tasks_data_insert";


DBTaskData::DBTaskData():
   af::TaskData()
{
   addDBAttributes();
}

DBTaskData::DBTaskData( af::Msg * msg)
{
//printf("DBTaskData::DBTaskData( af::Msg * msg):\n");
   addDBAttributes();
   read( msg);
}

void DBTaskData::addDBAttributes()
{
   dbAddAttr( new DBAttrString( DBAttr::_name,        &name       ));
   dbAddAttr( new DBAttrString( DBAttr::_command,     &command    ));
   dbAddAttr( new DBAttrString( DBAttr::_files,       &files      ));
   dbAddAttr( new DBAttrString( DBAttr::_dependmask,  &dependmask ));
   dbAddAttr( new DBAttrString( DBAttr::_customdata,  &customdata ));
}

DBTaskData::~DBTaskData()
{
AFINFO("DBTaskData::~DBTaskData")
}

const std::string DBTaskData::dbWhereSelect( int id_job, int id_block, int id_task)
{
   return std::string("id_job=") + af::itos(id_job) + " AND id_block=" + af::itos(id_block) + " AND id_task=" + af::itos(id_task);
}

bool DBTaskData::dbPrepareInsert( PGconn * i_conn)
{
    PGresult * res = PQprepare( i_conn, ms_db_prepare_name,
    "INSERT INTO tasks"
    " (id_job,id_block,id_task,name,command,files,dependmask,customdata)"
    " VALUES($1,$2,$3,$4,$5,$6,$7,$8);",
    8, NULL);
    bool o_result = true;
    if( PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        AFERRAR("SQL Preparing tasks data insertion failed: %s", PQerrorMessage( i_conn));
        o_result = false;
    }
    PQclear( res);
    return o_result;
}

bool DBTaskData::dbPrepareInsertExec( int id_job, int id_block, int id_task, PGconn * i_conn) const
{
    std::string id_job_str(   af::itos( id_job    ));
    std::string id_block_str( af::itos( id_block  ));
    std::string id_task_str(  af::itos( id_task   ));

    const char * data_pointers[8];
    data_pointers[0] = id_job_str.c_str();
    data_pointers[1] = id_block_str.c_str();
    data_pointers[2] = id_task_str.c_str();
    data_pointers[3] = name.c_str();
    data_pointers[4] = command.c_str();
    data_pointers[5] = files.c_str();
    data_pointers[6] = dependmask.c_str();
    data_pointers[7] = customdata.c_str();

    PGresult * res = PQexecPrepared( i_conn, ms_db_prepare_name,
                                     8, data_pointers,
                                     NULL, NULL, 0);
    bool o_result = true;
    if( PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        AFERRAR("SQL Executing prepared tasks data insertion failed: %s", PQerrorMessage( i_conn));
        o_result = false;
    }
    PQclear( res);
    return o_result;
}

void DBTaskData::readwrite( af::Msg * msg)
{
//printf("DBTaskData::readwrite:\n");
   if( msg->isReading()) af::TaskData::readwrite(msg);
   else
   {
      static bool name_only = true;
      rw_bool(    name_only,    msg);
      rw_String(  name,         msg);
   }
}
