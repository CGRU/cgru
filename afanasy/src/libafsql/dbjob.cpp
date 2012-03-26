#include "dbjob.h"

#include "dbattr.h"
#include "dbblockdata.h"
#include "dbjobprogress.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afsql;

const std::string DBJob::TableName("jobs");
const std::string DBJob::Keys("PRIMARY KEY( id)");
const int         DBJob::KeysNum = 1;
DBStatistics      DBJob::statistics;

DBJob::DBJob( int Id):
   af::Job( Id),
   progress( NULL)
{
   addDBAttributes();
}

void DBJob::addDBAttributes()
{
   dbAddAttr( new DBAttrInt32(  DBAttr::_id,                 &id                  ));

   dbAddAttr( new DBAttrUInt32( DBAttr::_state,              &state               ));
   dbAddAttr( new DBAttrUInt32( DBAttr::_flags,              &flags               ));
   dbAddAttr( new DBAttrUInt8 ( DBAttr::_priority,           &priority            ));
   dbAddAttr( new DBAttrInt64 ( DBAttr::_time_started,       &time_started        ));
   dbAddAttr( new DBAttrInt64 ( DBAttr::_time_done,          &time_done           ));
   dbAddAttr( new DBAttrInt64 ( DBAttr::_time_wait,          &time_wait           ));
   dbAddAttr( new DBAttrInt32 ( DBAttr::_maxrunningtasks,    &maxrunningtasks     ));
   dbAddAttr( new DBAttrInt32 ( DBAttr::_maxruntasksperhost, &maxruntasksperhost  ));
   dbAddAttr( new DBAttrInt32 ( DBAttr::_userlistorder,      &userlistorder       ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_hostsmask,          &hostsmask           ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_hostsmask_exclude,  &hostsmask_exclude   ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_dependmask,         &dependmask          ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_dependmask_global,  &dependmask_global   ));
   dbAddAttr( new DBAttrInt32 ( DBAttr::_lifetime,           &lifetime            ));
   dbAddAttr( new DBAttrString( DBAttr::_annotation,         &annotation          ));
   dbAddAttr( new DBAttrString( DBAttr::_cmd_post,           &cmd_post            ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_need_os,            &need_os             ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_need_properties,    &need_properties     ));
   dbAddAttr( new DBAttrString( DBAttr::_description,        &description         ));

   dbAddAttr( new DBAttrString( DBAttr::_name,               &name                ));
   dbAddAttr( new DBAttrString( DBAttr::_hostname,           &hostname            ));
   dbAddAttr( new DBAttrString( DBAttr::_username,           &username            ));
   dbAddAttr( new DBAttrInt32 ( DBAttr::_blocksnum,          &blocksnum           ));
   dbAddAttr( new DBAttrString( DBAttr::_cmd_pre,            &cmd_pre             ));
   dbAddAttr( new DBAttrInt64 ( DBAttr::_time_creation,      &time_creation       ));
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

const std::string DBJob::dbGetIDsCmd()
{
   return std::string("SELECT id FROM ") + TableName + " ORDER BY userlistorder";
}

bool DBJob::dbAdd( PGconn * i_conn) const
{
    AFINFA("DBJob::dbAdd: name = '%s', id = %d", name.c_str(), id);

    std::list<std::string> queries;
    dbInsert( &queries);
    if( false == execute( i_conn, &queries))
    {
        AFERROR("Adding job to database failed.\n")
        return false;
    }

    bool o_result = true;

    for( int b = 0; b < blocksnum; b++)
    {
        if( false == ((DBBlockData*)(blocksdata[b]))->dbAdd( i_conn))
        {
            o_result = false;
            break;
        }
    }

    if( o_result )
    {
        o_result = progress->dbAdd( i_conn);
    }

    if( false == o_result )
    {
        std::list<std::string> queries;
        dbDeleteNoStatistics( &queries);
        execute( i_conn, &queries);
    }

    return o_result;
}

bool DBJob::dbSelect( PGconn * i_conn, const std::string * i_where)
{
    AFINFA("DBJob::dbSelect: id = %d", id);

   if( DBItem::dbSelect( i_conn) == false) return false;
   if( blocksnum == 0)
   {
      AFERROR("DBJob::dbSelect: blocksnum == 0")
      return false;
   }
   blocksdata = new af::BlockData*[blocksnum];
   for( int b = 0; b < blocksnum; b++) blocksdata[b] = NULL;
   for( int b = 0; b < blocksnum; b++)
   {
      DBBlockData * dbBlock = new DBBlockData( b, id);
      if( dbBlock->dbSelect( i_conn) == false)
      {
         delete dbBlock;
         return false;
      }
      blocksdata[b] = dbBlock;
   }
   progress = new DBJobProgress( this);
   if( progress == NULL)
   {
      AFERROR("DBJob::dbSelect: can't allocate memory for tasks progress.")
      return false;
   }
   if( progress->dbSelect( i_conn) == false) return false;

   return true;
}

void DBJob::dbDelete( std::list<std::string> * queries) const
{
   DBItem::dbDelete( queries);
   if( id != AFJOB::SYSJOB_ID) // Do not add system job to statistics
      statistics.addJob( this, queries);
}

void DBJob::dbDeleteNoStatistics( std::list<std::string> * queries) const
{
   DBItem::dbDelete( queries);
}
