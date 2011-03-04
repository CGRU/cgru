#include "dbjob.h"

#include "dbattr.h"
#include "dbblockdata.h"
#include "dbjobprogress.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afsql;

const QString DBJob::TableName("jobs");
const QString DBJob::Keys("PRIMARY KEY( id)");
const int     DBJob::KeysNum = 1;
DBStatistics  DBJob::statistics;

DBJob::DBJob( int Id):
   af::Job( Id),
   progress( NULL)
{
   addDBAttributes();
}

void DBJob::addDBAttributes()
{
   dbAddAttr( new DBAttrInt32(  DBAttr::_id,                 &id                  ));

   dbAddAttr( new DBAttrUInt32(  DBAttr::_state,              &state               ));
   dbAddAttr( new DBAttrUInt8 (  DBAttr::_priority,           &priority            ));
   dbAddAttr( new DBAttrUInt32(  DBAttr::_time_started,       &time_started        ));
   dbAddAttr( new DBAttrUInt32(  DBAttr::_time_done,          &time_done           ));
   dbAddAttr( new DBAttrUInt32(  DBAttr::_time_wait,          &time_wait           ));
   dbAddAttr( new DBAttrInt32 (  DBAttr::_maxrunningtasks,    &maxrunningtasks     ));
   dbAddAttr( new DBAttrInt32 (  DBAttr::_userlistorder,      &userlistorder       ));
   dbAddAttr( new DBAttrQRegExp( DBAttr::_hostsmask,          &hostsmask           ));
   dbAddAttr( new DBAttrQRegExp( DBAttr::_hostsmask_exclude,  &hostsmask_exclude   ));
   dbAddAttr( new DBAttrQRegExp( DBAttr::_dependmask,         &dependmask          ));
   dbAddAttr( new DBAttrQRegExp( DBAttr::_dependmask_global,  &dependmask_global   ));
   dbAddAttr( new DBAttrInt32 (  DBAttr::_lifetime,           &lifetime            ));
   dbAddAttr( new DBAttrString(  DBAttr::_annotation,         &annotation          ));
   dbAddAttr( new DBAttrString(  DBAttr::_cmd_post,           &cmd_post            ));
   dbAddAttr( new DBAttrQRegExp( DBAttr::_need_os,            &need_os             ));
   dbAddAttr( new DBAttrQRegExp( DBAttr::_need_properties,    &need_properties     ));
   dbAddAttr( new DBAttrString(  DBAttr::_description,        &description         ));
   dbAddAttr( new DBAttrUInt32(  DBAttr::_flags,              &flags               ));

   dbAddAttr( new DBAttrString(  DBAttr::_name,               &name                ));
   dbAddAttr( new DBAttrString(  DBAttr::_hostname,           &hostname            ));
   dbAddAttr( new DBAttrString(  DBAttr::_username,           &username            ));
   dbAddAttr( new DBAttrInt32 (  DBAttr::_blocksnum,          &blocksnum           ));
   dbAddAttr( new DBAttrString(  DBAttr::_cmd_pre,            &cmd_pre             ));
   dbAddAttr( new DBAttrUInt32(  DBAttr::_time_creation,      &time_creation       ));
}

DBJob::~DBJob()
{
   if( progress != NULL ) delete progress;
}

af::BlockData * DBJob::newBlockData( af::Msg * msg)
{
//printf("DBJob::createBlock:\n");
   return new DBBlockData( msg);
}

void DBJob::getIds( std::list<int32_t> & uids, QSqlDatabase * db)
{
   if( db->isOpen() == false )
   {
      AFERROR("DBJob::getIds: Database connection is not open\n");
      return;
   }
   QSqlQuery q( *db);
   q.exec(QString("SELECT id FROM %1 ORDER BY userlistorder").arg( TableName));
   while (q.next()) uids.push_back( q.value(0).toUInt());
printf("DBJob::getIds: %u jobs founded.\n", unsigned(uids.size()));
}

void DBJob::dbAdd( QSqlDatabase * db) const
{
   QStringList queries;
   dbInsert( &queries);
   QSqlQuery q( *db);
   for( int i = 0; i < queries.size(); i++) q.exec( queries[i]);

   for( int b = 0; b < blocksnum; b++) ((DBBlockData*)(blocksdata[b]))->dbAdd( db);

   progress->dbAdd( db);
}

bool DBJob::dbSelect( QSqlDatabase * db, const QString * where)
{
//printf("DBJob::dbSelect:\n");
   if( DBItem::dbSelect( db) == false) return false;
   if( blocksnum == 0)
   {
      AFERROR("DBJob::dbSelect: blocksnum == 0\n");
      return false;
   }
   blocksdata = new af::BlockData*[blocksnum];
   for( int b = 0; b < blocksnum; b++) blocksdata[b] = NULL;
   for( int b = 0; b < blocksnum; b++)
   {
      DBBlockData * dbBlock = new DBBlockData( b, id);
      if( dbBlock->dbSelect( db) == false)
      {
         delete dbBlock;
         return false;
      }
      blocksdata[b] = dbBlock;
   }
   progress = new DBJobProgress( this);
   if( progress == NULL)
   {
      AFERROR("DBJob::dbSelect: can't allocate memory for tasks progress.\n");
      return false;
   }
   if( progress->dbSelect( db) == false) return false;

   return true;
}

void DBJob::dbDelete( QStringList  * queries) const
{
   DBItem::dbDelete( queries);
   statistics.addJob( this, queries);
}
