#include "dbjob.h"

#include "../libafanasy/job.h"

#include "dbattr.h"

using namespace afsql;

const std::string DBJob::ms_TableName("jobs");

DBJob::DBJob()
{
	dbAddAttr( new DBAttrString( DBAttr::_annotation,     &m_annotation     ));
	dbAddAttr( new DBAttrString( DBAttr::_blockname,      &m_blockname      ));
	dbAddAttr( new DBAttrInt32 ( DBAttr::_capacity,       &m_capacity       ));
	dbAddAttr( new DBAttrString( DBAttr::_description,    &m_description    ));
	dbAddAttr( new DBAttrString( DBAttr::_folder,         &m_folder         ));
	dbAddAttr( new DBAttrString( DBAttr::_jobname,        &m_jobname        ));
	dbAddAttr( new DBAttrString( DBAttr::_hostname,       &m_hostname       ));
	dbAddAttr( new DBAttrString( DBAttr::_service,        &m_service        ));
	dbAddAttr( new DBAttrInt32 ( DBAttr::_tasks_done,     &m_tasks_done     ));
	dbAddAttr( new DBAttrInt32 ( DBAttr::_tasks_quantity, &m_tasks_quantity ));
	dbAddAttr( new DBAttrInt64 ( DBAttr::_run_time_sum,   &m_run_time_sum   ));
	dbAddAttr( new DBAttrInt64 ( DBAttr::_time_done,      &m_time_done      ));
	dbAddAttr( new DBAttrInt64 ( DBAttr::_time_started,   &m_time_start     ));
	dbAddAttr( new DBAttrString( DBAttr::_username,       &m_username       ));
}

DBJob::~DBJob()
{
}

void DBJob::add( const af::Job * i_job, std::list<std::string> * o_queries)
{
	// Get job parameters:
	m_jobname     = i_job->getName();
	m_description = i_job->getDescription();
	m_folder      = i_job->getFolder();
	m_annotation  = i_job->getAnnotation();
	m_username    = i_job->getUserName();
	m_hostname    = i_job->getHostName();
	m_time_start  = i_job->getTimeStarted();
	m_time_done   = i_job->getTimeDone();

	// Skip not started i_job:
	if( m_time_start == 0 ) return;
	// Skip job with no running time:
	if( m_time_start == m_time_done ) return;
	// Set done time to current time, if job was not done:
	if( m_time_done < m_time_start ) m_time_done = time( NULL);

	// Inserting each block in table:
	for( int b = 0; b < i_job->getBlocksNum(); b++)
	{
		// Get block parameters:
		af::BlockData * block = i_job->getBlock(b);
		m_blockname = block->getName();
		m_capacity = block->getCapacity();
		m_service = block->getService();
		m_tasks_quantity = block->getTasksNum();
		m_tasks_done = block->getProgressTasksDone();
		m_run_time_sum = block->getProgressTasksSumRunTime();

		// Skip blocks with no run time:
		if( m_tasks_quantity == 0 ) continue;
		if( m_tasks_done == 0 ) continue;
		if( m_run_time_sum == 0 ) continue;

		// Insert row:
		v_dbInsert( o_queries);
	}
}
