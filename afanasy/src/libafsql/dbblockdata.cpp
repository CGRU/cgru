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
   addDBAttributes();
   read( msg);
}

DBBlockData::DBBlockData( int BlockNum, int JobId):
   af::BlockData( BlockNum, JobId)
{
AFINFA("DBBlockData::DBBlockData(): JobId=%d, BlockNum=%d", jobid, blocknum)
   addDBAttributes();
}

DBBlockData::DBBlockData():
   af::BlockData( 0, 0)
{
   addDBAttributes();
}

void DBBlockData::addDBAttributes()
{
   dbAddAttr( new DBAttrInt32Const(          DBAttr::_id_job, &jobid                  ));

   dbAddAttr( new DBAttrInt32(  DBAttr::_id_block,             &blocknum               ));

   dbAddAttr( new DBAttrString( DBAttr::_command,              &command                ));
   dbAddAttr( new DBAttrUInt32( DBAttr::_tasksmaxruntime,      &tasksmaxruntime        ));
   dbAddAttr( new DBAttrInt8(   DBAttr::_errors_retries,       &errors_retries         ));
   dbAddAttr( new DBAttrInt8(   DBAttr::_errors_avoidhost,     &errors_avoidhost       ));
   dbAddAttr( new DBAttrInt8(   DBAttr::_errors_tasksamehost,  &errors_tasksamehost    ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_errors_forgivetime,   &errors_forgivetime     ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_capacity,             &capacity               ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_capcoeff_min,         &capcoeff_min           ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_capcoeff_max,         &capcoeff_max           ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_maxrunningtasks,      &maxrunningtasks        ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_maxruntasksperhost,   &maxruntasksperhost     ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_dependmask,           &dependmask             ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_tasksdependmask,      &tasksdependmask        ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_hostsmask,            &hostsmask              ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_hostsmask_exclude,    &hostsmask_exclude      ));
   dbAddAttr( new DBAttrInt64(  DBAttr::_filesize_min,         &filesize_min           ));
   dbAddAttr( new DBAttrInt64(  DBAttr::_filesize_max,         &filesize_max           ));
   dbAddAttr( new DBAttrUInt8(  DBAttr::_multihost_min,        &multihost_min          ));
   dbAddAttr( new DBAttrUInt8(  DBAttr::_multihost_max,        &multihost_max          ));
   dbAddAttr( new DBAttrUInt16( DBAttr::_multihost_waitmax,    &multihost_waitmax      ));
   dbAddAttr( new DBAttrUInt16( DBAttr::_multihost_waitsrv,    &multihost_waitsrv      ));
   dbAddAttr( new DBAttrString( DBAttr::_wdir,                 &wdir                   ));
   dbAddAttr( new DBAttrString( DBAttr::_files,                &files                  ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_need_properties,      &need_properties        ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_need_memory,          &need_memory            ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_need_hdd,             &need_hdd               ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_need_power,           &need_power             ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_parsercoeff,          &parsercoeff            ));
   dbAddAttr( new DBAttrString( DBAttr::_service,              &service                ));
   dbAddAttr( new DBAttrString( DBAttr::_parser,               &parser                 ));
   dbAddAttr( new DBAttrString( DBAttr::_cmd_post,             &cmd_post               ));
   dbAddAttr( new DBAttrString( DBAttr::_environment,          &environment            ));
   dbAddAttr( new DBAttrString( DBAttr::_customdata,           &customdata             ));

   dbAddAttr( new DBAttrString( DBAttr::_name,                 &name                   ));
   dbAddAttr( new DBAttrString( DBAttr::_cmd_pre,              &cmd_pre                ));
   dbAddAttr( new DBAttrUInt32( DBAttr::_flags,                &flags                  ));
   dbAddAttr( new DBAttrInt64(  DBAttr::_frame_pertask,        &frame_pertask          ));
   dbAddAttr( new DBAttrInt64(  DBAttr::_frame_first,          &frame_first            ));
   dbAddAttr( new DBAttrInt64(  DBAttr::_frame_last,           &frame_last             ));
   dbAddAttr( new DBAttrInt64(  DBAttr::_frame_inc,            &frame_inc              ));
   dbAddAttr( new DBAttrString( DBAttr::_multihost_service,    &multihost_service      ));
   dbAddAttr( new DBAttrString( DBAttr::_tasksname,            &tasksname              ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_tasksnum,             &tasksnum               ));
}

DBBlockData::~DBBlockData()
{
}

af::TaskData * DBBlockData::createTask( af::Msg * msg)
{
//printf("DBBlockData::createTask:\n");
   return new DBTaskData( msg);
}

bool DBBlockData::dbAdd( PGconn * i_conn) const
{
AFINFA("DBBlockData::dbAdd: blocknum = %d, tasksnum = %d", blocknum, tasksnum)
   std::list<std::string> queries;
   dbInsert( &queries);

   if( false == execute( i_conn, &queries))
   {
       AFERROR("Failed to add job block to database.\n");
       return false;
   }

   if( isNumeric())
   {
       return true;
   }

   DBTaskData::dbPrepareInsert( i_conn);
   for( int t = 0; t < tasksnum; t++)
   {
      if( false == ((DBTaskData*)(tasksdata[t]))->dbPrepareInsertExec( jobid, blocknum, t, i_conn))
      {
          AFERROR("Failed to add task data to database.\n")
          return false;
      }
   }

   return true;
}

bool DBBlockData::dbSelect( PGconn * i_conn, const std::string * i_where)
{
   if( DBItem::dbSelect( i_conn) == false) return false;
   if( isNumeric() || (tasksnum == 0)) return true;

   tasksdata = new af::TaskData*[tasksnum];
   for( int t = 0; t < tasksnum; t++) tasksdata[t] = NULL;
   for( int t = 0; t < tasksnum; t++)
   {
      DBTaskData * dbtaskdata = new DBTaskData;
      std::string where = DBTaskData::dbWhereSelect( jobid, blocknum, t);
      if( dbtaskdata->dbSelect( i_conn, &where) == false)
      {
         delete dbtaskdata;
         return false;
      }
      tasksdata[t] = dbtaskdata;
   }

   return true;
}
