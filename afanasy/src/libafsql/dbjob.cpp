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
AFINFA("DBJob::DBJob: id=%d", Id)
   addDBAttributes();
}

void DBJob::addDBAttributes()
{
   dbAddAttr( new DBAttrInt32(  DBAttr::_id,                 &m_id                  ));

   dbAddAttr( new DBAttrUInt32( DBAttr::_state,              &m_state               ));
   dbAddAttr( new DBAttrUInt32( DBAttr::_flags,              &m_flags               ));
   dbAddAttr( new DBAttrUInt8 ( DBAttr::_priority,           &m_priority            ));
   dbAddAttr( new DBAttrInt64 ( DBAttr::_time_started,       &m_time_started        ));
   dbAddAttr( new DBAttrInt64 ( DBAttr::_time_done,          &m_time_done           ));
   dbAddAttr( new DBAttrInt64 ( DBAttr::_time_wait,          &m_time_wait           ));
   dbAddAttr( new DBAttrInt32 ( DBAttr::_maxrunningtasks,    &m_max_running_tasks     ));
   dbAddAttr( new DBAttrInt32 ( DBAttr::_maxruntasksperhost, &m_max_running_tasks_per_host  ));
   dbAddAttr( new DBAttrInt32 ( DBAttr::_userlistorder,      &m_user_list_order       ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_hostsmask,          &m_hosts_mask           ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_hostsmask_exclude,  &m_hosts_mask_exclude   ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_dependmask,         &m_depend_mask          ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_dependmask_global,  &m_depend_mask_global   ));
   dbAddAttr( new DBAttrInt32 ( DBAttr::_lifetime,           &m_time_life            ));
   dbAddAttr( new DBAttrString( DBAttr::_annotation,         &m_annotation          ));
   dbAddAttr( new DBAttrString( DBAttr::_cmd_post,           &m_command_post            ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_need_os,            &m_need_os             ));
   dbAddAttr( new DBAttrRegExp( DBAttr::_need_properties,    &m_need_properties     ));
   dbAddAttr( new DBAttrString( DBAttr::_description,        &m_description         ));
	dbAddAttr( new DBAttrString( DBAttr::_customdata,         &m_custom_data         ));

   dbAddAttr( new DBAttrString( DBAttr::_name,               &m_name                ));
   dbAddAttr( new DBAttrString( DBAttr::_hostname,           &m_host_name            ));
   dbAddAttr( new DBAttrString( DBAttr::_username,           &m_user_name            ));
   dbAddAttr( new DBAttrInt32 ( DBAttr::_blocksnum,          &m_blocksnum           ));
   dbAddAttr( new DBAttrString( DBAttr::_cmd_pre,            &m_command_pre             ));
   dbAddAttr( new DBAttrInt64 ( DBAttr::_time_creation,      &m_time_creation       ));
}

DBJob::~DBJob()
{
	if( progress != NULL ) delete progress;
}

af::BlockData * DBJob::v_newBlockData( af::Msg * msg)
{
//printf("DBJob::createBlock:\n");
	return new DBBlockData( msg);
}

af::BlockData * DBJob::v_newBlockData( const JSON & i_object, int i_num)
{
	return new DBBlockData( i_object, i_num);
}

const std::string DBJob::dbGetIDsCmd()
{
	return std::string("SELECT id FROM ") + TableName + " ORDER BY userlistorder";
}

bool DBJob::dbAdd( PGconn * i_conn) const
{
    AFINFA("DBJob::dbAdd: name = '%s', id = %d", m_name.c_str(), m_id);

    std::list<std::string> queries;
    v_dbInsert( &queries);
    if( false == execute( i_conn, &queries))
    {
        AFERROR("Adding job to database failed.\n")
        return false;
    }

    bool o_result = true;

    for( int b = 0; b < m_blocksnum; b++)
    {
        if( false == ((DBBlockData*)(m_blocksdata[b]))->dbAdd( i_conn))
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

bool DBJob::v_dbSelect( PGconn * i_conn, const std::string * i_where)
{
    AFINFA("DBJob::dbSelect: id = %d", m_id);

   if( DBItem::v_dbSelect( i_conn) == false) return false;
   if( m_blocksnum == 0)
   {
      AFERROR("DBJob::dbSelect: blocksnum == 0")
      return false;
   }
   m_blocksdata = new af::BlockData*[m_blocksnum];
   for( int b = 0; b < m_blocksnum; b++) m_blocksdata[b] = NULL;
   for( int b = 0; b < m_blocksnum; b++)
   {
      DBBlockData * dbBlock = new DBBlockData( b, m_id);
      if( dbBlock->v_dbSelect( i_conn) == false)
      {
         delete dbBlock;
         return false;
      }
      m_blocksdata[b] = dbBlock;
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

void DBJob::v_dbDelete( std::list<std::string> * queries) const
{
   DBItem::v_dbDelete( queries);
   if( m_id != AFJOB::SYSJOB_ID) // Do not add system job to statistics
      statistics.addJob( this, queries);
}

void DBJob::dbDeleteNoStatistics( std::list<std::string> * queries) const
{
   DBItem::v_dbDelete( queries);
}
