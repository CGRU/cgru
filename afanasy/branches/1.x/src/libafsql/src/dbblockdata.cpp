#include "dbblockdata.h"

#include "dbattr.h"
#include "dbtaskdata.h"

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

using namespace afsql;

const QString DBBlockData::TableName("blocks");
const QString DBBlockData::Keys("FOREIGN KEY (id_job) REFERENCES jobs(id) ON DELETE CASCADE, PRIMARY KEY (id_job, id_block)");
const int     DBBlockData::KeysNum = 2;

DBBlockData::DBBlockData( af::Msg * msg):
   af::BlockData( 0, 0)
{
   addDBAttributes();
   read( msg);
}

DBBlockData::DBBlockData( int BlockNum, int JobId):
   af::BlockData( BlockNum, JobId)
{
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

   dbAddAttr( new DBAttrString( DBAttr::_cmd,                  &cmd                    ));
   dbAddAttr( new DBAttrUInt32( DBAttr::_tasksmaxruntime,      &tasksmaxruntime        ));
   dbAddAttr( new DBAttrInt8(   DBAttr::_errors_retries,       &errors_retries         ));
   dbAddAttr( new DBAttrInt8(   DBAttr::_errors_avoidhost,     &errors_avoidhost       ));
   dbAddAttr( new DBAttrInt8(   DBAttr::_errors_tasksamehost,  &errors_tasksamehost    ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_capacity,             &capacity               ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_capcoeff_min,         &capcoeff_min           ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_capcoeff_max,         &capcoeff_max           ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_maxhosts,             &maxhosts               ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_dependmask,           &dependmask             ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_tasksdependmask,      &tasksdependmask        ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_hostsmask,            &hostsmask              ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_hostsmask_exclude,    &hostsmask_exclude      ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_filesize_min,         &filesize_min           ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_filesize_max,         &filesize_max           ));
   dbAddAttr( new DBAttrUInt8(  DBAttr::_multihost_min,        &multihost_min          ));
   dbAddAttr( new DBAttrUInt8(  DBAttr::_multihost_max,        &multihost_max          ));
   dbAddAttr( new DBAttrUInt16( DBAttr::_multihost_waitmax,    &multihost_waitmax      ));
   dbAddAttr( new DBAttrUInt16( DBAttr::_multihost_waitsrv,    &multihost_waitsrv      ));
   dbAddAttr( new DBAttrString( DBAttr::_wdir,                 &wdir                   ));
   dbAddAttr( new DBAttrString( DBAttr::_cmd_view,             &cmd_view               ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_need_properties,      &need_properties        ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_need_memory,          &need_memory            ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_need_hdd,             &need_hdd               ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_need_power,           &need_power             ));
   dbAddAttr( new DBAttrString( DBAttr::_parsertype,           &parsertype             ));
   dbAddAttr( new DBAttrString( DBAttr::_taskstype,            &taskstype              ));
   dbAddAttr( new DBAttrString( DBAttr::_cmd_post,             &cmd_post               ));
   dbAddAttr( new DBAttrString( DBAttr::_environment,          &environment            ));

   dbAddAttr( new DBAttrString( DBAttr::_name,                 &name                   ));
   dbAddAttr( new DBAttrString( DBAttr::_cmd_pre,              &cmd_pre                ));
   dbAddAttr( new DBAttrUInt32( DBAttr::_flags,                &flags                  ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_frame_perhost,        &frame_perhost          ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_frame_first,          &frame_first            ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_frame_last,           &frame_last             ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_frame_inc,            &frame_inc              ));
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

void DBBlockData::dbAdd( QSqlDatabase * db) const
{
AFINFA("DBBlockData::dbAdd: tasksnum = %d\n", tasksnum);
   QStringList queries;
   dbInsert( &queries);
   QSqlQuery q( *db);
   for( int i = 0; i < queries.size(); i++) q.exec( queries[i]);
   if( isNumeric()) return;

   q.prepare( DBTaskData::dbPrepareInsert);
   QVariant job( jobid);
   QVariant block( blocknum);
   for( int t = 0; t < tasksnum; t++)
   {
      ((DBTaskData*)(tasksdata[t]))->dbBindInsert( &q, job, block, QVariant(t));
      q.exec();
      if( qChkErr(q, "DBBlockData::dbAdd:\n")) break;
   }
}

bool DBBlockData::dbSelect( QSqlDatabase * db, const QString * where)
{
   if( DBItem::dbSelect( db) == false) return false;
   if( isNumeric() || (tasksnum == 0)) return true;

   tasksdata = new af::TaskData*[tasksnum];
   for( int t = 0; t < tasksnum; t++) tasksdata[t] = NULL;
   for( int t = 0; t < tasksnum; t++)
   {
      DBTaskData * dbtaskdata = new DBTaskData;
      QString where = DBTaskData::dbWhereSelect( jobid, blocknum, t);
      if( dbtaskdata->dbSelect( db, &where) == false)
      {
         delete dbtaskdata;
         return false;
      }
      tasksdata[t] = dbtaskdata;
   }

   return true;
}
