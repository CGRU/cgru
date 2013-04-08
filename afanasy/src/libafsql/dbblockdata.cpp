#include "dbblockdata.h"

#include "dbattr.h"
#include "dbtaskdata.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afsql;

const std::string DBBlockData::TableName("blocks");
const std::string DBBlockData::Keys("FOREIGN KEY (id_job) REFERENCES jobs(id) ON DELETE CASCADE, PRIMARY KEY (id_job, id_block)");
const int         DBBlockData::KeysNum = 2;

DBBlockData::DBBlockData( af::Msg * msg):
   af::BlockData( 0, 0)
{
AFINFO("DBBlockData::DBBlockData():")
   addDBAttributes();
   read( msg);
}

DBBlockData::DBBlockData( const JSON & i_object, int i_num):
   af::BlockData( i_num, 0)
{
AFINFA("DBBlockData::DBBlockData(): BlockNum=%d", i_num)
   addDBAttributes();
   jsonRead( i_object);
}

DBBlockData::DBBlockData( int BlockNum, int JobId):
   af::BlockData( BlockNum, JobId)
{
AFINFA("DBBlockData::DBBlockData(): JobId=%d, BlockNum=%d", m_job_id, m_block_num)
   addDBAttributes();
}

DBBlockData::DBBlockData():
   af::BlockData( 0, 0)
{
   addDBAttributes();
}

void DBBlockData::addDBAttributes()
{
   dbAddAttr( new DBAttrInt32Const(          DBAttr::_id_job, &m_job_id                  ));

   dbAddAttr( new DBAttrInt32(  DBAttr::_id_block,             &m_block_num               ));

   dbAddAttr( new DBAttrString( DBAttr::_command,              &m_command                ));
   dbAddAttr( new DBAttrUInt32( DBAttr::_tasksmaxruntime,      &m_tasks_max_run_time        ));
   dbAddAttr( new DBAttrInt8(   DBAttr::_errors_retries,       &m_errors_retries         ));
   dbAddAttr( new DBAttrInt8(   DBAttr::_errors_avoidhost,     &m_errors_avoid_host       ));
   dbAddAttr( new DBAttrInt8(   DBAttr::_errors_tasksamehost,  &m_errors_task_same_host    ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_errors_forgivetime,   &m_errors_forgive_time     ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_capacity,             &m_capacity               ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_capcoeff_min,         &m_capacity_coeff_min           ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_capcoeff_max,         &m_capacity_coeff_max           ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_maxrunningtasks,      &m_max_running_tasks        ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_maxruntasksperhost,   &m_max_running_tasks_per_host     ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_dependmask,           &m_depend_mask             ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_tasksdependmask,      &m_tasks_depend_mask        ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_hostsmask,            &m_hosts_mask              ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_hostsmask_exclude,    &m_hosts_mask_exclude      ));
   dbAddAttr( new DBAttrInt64(  DBAttr::_filesize_min,         &m_file_size_min           ));
   dbAddAttr( new DBAttrInt64(  DBAttr::_filesize_max,         &m_file_size_max           ));
   dbAddAttr( new DBAttrUInt8(  DBAttr::_multihost_min,        &m_multihost_min          ));
   dbAddAttr( new DBAttrUInt8(  DBAttr::_multihost_max,        &m_multihost_max          ));
   dbAddAttr( new DBAttrUInt16( DBAttr::_multihost_waitmax,    &m_multihost_max_wait      ));
   dbAddAttr( new DBAttrUInt16( DBAttr::_multihost_waitsrv,    &m_multihost_service_wait      ));
   dbAddAttr( new DBAttrString( DBAttr::_wdir,                 &m_working_directory                   ));
   dbAddAttr( new DBAttrString( DBAttr::_files,                &m_files                  ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_need_properties,      &m_need_properties        ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_need_memory,          &m_need_memory            ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_need_hdd,             &m_need_hdd               ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_need_power,           &m_need_power             ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_parsercoeff,          &m_parser_coeff            ));
   dbAddAttr( new DBAttrString( DBAttr::_service,              &m_service                ));
   dbAddAttr( new DBAttrString( DBAttr::_parser,               &m_parser                 ));
   dbAddAttr( new DBAttrString( DBAttr::_cmd_post,             &m_command_post               ));
   dbAddAttr( new DBAttrString( DBAttr::_environment,          &m_environment            ));
   dbAddAttr( new DBAttrString( DBAttr::_customdata,           &m_custom_data             ));

   dbAddAttr( new DBAttrString( DBAttr::_name,                 &m_name                   ));
   dbAddAttr( new DBAttrString( DBAttr::_cmd_pre,              &m_command_pre                ));
   dbAddAttr( new DBAttrUInt32( DBAttr::_flags,                &m_flags                  ));
   dbAddAttr( new DBAttrInt64(  DBAttr::_frame_pertask,        &m_frames_per_task          ));
   dbAddAttr( new DBAttrInt64(  DBAttr::_frame_first,          &m_frame_first            ));
   dbAddAttr( new DBAttrInt64(  DBAttr::_frame_last,           &m_frame_last             ));
   dbAddAttr( new DBAttrInt64(  DBAttr::_frame_inc,            &m_frames_inc              ));
   dbAddAttr( new DBAttrString( DBAttr::_multihost_service,    &m_multihost_service      ));
   dbAddAttr( new DBAttrString( DBAttr::_tasksname,            &m_tasks_name              ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_tasksnum,             &m_tasks_num               ));
}

DBBlockData::~DBBlockData()
{
}

af::TaskData * DBBlockData::createTask( af::Msg * msg)
{
//printf("DBBlockData::createTask:\n");
   return new DBTaskData( msg);
}

af::TaskData * DBBlockData::createTask( const JSON & i_object)
{
	return new DBTaskData( i_object);
}

bool DBBlockData::dbAdd( PGconn * i_conn) const
{
AFINFA("DBBlockData::dbAdd: %s[%d,%d] %d tasks %s", m_name.c_str(), m_job_id, m_block_num, m_tasks_num, isNumeric() ? "NUMERIC":"NOT_NUMERIC")
   std::list<std::string> queries;
   v_dbInsert( &queries);

   if( false == execute( i_conn, &queries))
   {
       AFERROR("Failed to add job block to database.\n");
       return false;
   }

   if( isNumeric())
   {
       return true;
   }

	// Prepare query, but only for the first block (to do it at one per connection)
	if( m_block_num == 0 )
		DBTaskData::dbPrepareInsert( i_conn);

   for( int t = 0; t < m_tasks_num; t++)
   {
      if( false == ((DBTaskData*)(m_tasks_data[t]))->dbPrepareInsertExec( m_job_id, m_block_num, t, i_conn))
      {
          AFERROR("Failed to add task data to database.\n")
          return false;
      }
   }

   return true;
}

bool DBBlockData::v_dbSelect( PGconn * i_conn, const std::string * i_where)
{
   if( DBItem::v_dbSelect( i_conn) == false) return false;
   if( isNumeric() || (m_tasks_num == 0)) return true;

   m_tasks_data = new af::TaskData*[m_tasks_num];
   for( int t = 0; t < m_tasks_num; t++) m_tasks_data[t] = NULL;
   for( int t = 0; t < m_tasks_num; t++)
   {
      DBTaskData * dbtaskdata = new DBTaskData;
      std::string where = DBTaskData::dbWhereSelect( m_job_id, m_block_num, t);
      if( dbtaskdata->v_dbSelect( i_conn, &where) == false)
      {
         delete dbtaskdata;
         return false;
      }
      m_tasks_data[t] = dbtaskdata;
   }

   return true;
}
