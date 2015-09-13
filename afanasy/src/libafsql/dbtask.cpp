#include "dbtask.h"

#include "../libafanasy/job.h"
#include "../libafanasy/render.h"
#include "../libafanasy/taskexec.h"

#include "dbattr.h"

using namespace afsql;

const std::string DBTask::ms_TableName("tasks");

DBTask::DBTask()
{
	dbAddAttr( new DBAttrString( DBAttr::_annotation,   &m_annotation   ));
	dbAddAttr( new DBAttrString( DBAttr::_blockname,    &m_blockname    ));
	dbAddAttr( new DBAttrInt32 ( DBAttr::_capacity,     &m_capacity     ));
	dbAddAttr( new DBAttrString( DBAttr::_command,      &m_command      ));
	dbAddAttr( new DBAttrString( DBAttr::_description,  &m_description  ));
	dbAddAttr( new DBAttrInt32 ( DBAttr::_error,        &m_error        ));
	dbAddAttr( new DBAttrInt32 ( DBAttr::_errors_count, &m_errors_count ));
	dbAddAttr( new DBAttrString( DBAttr::_folder,       &m_folder       ));
	dbAddAttr( new DBAttrString( DBAttr::_hostname,     &m_hostname     ));
	dbAddAttr( new DBAttrString( DBAttr::_jobname,      &m_jobname      ));
	dbAddAttr( new DBAttrString( DBAttr::_service,      &m_service      ));
	dbAddAttr( new DBAttrInt32 ( DBAttr::_starts_count, &m_starts_count ));
	dbAddAttr( new DBAttrInt64 ( DBAttr::_time_done,    &m_time_done    ));
	dbAddAttr( new DBAttrInt64 ( DBAttr::_time_started, &m_time_start   ));
	dbAddAttr( new DBAttrString( DBAttr::_username,     &m_username     ));
}

DBTask::~DBTask()
{
}

void DBTask::add(
	const af::TaskExec * i_exec,
	const af::TaskProgress * i_progress,
	const af::Job * i_job,
	const af::Render * i_render,
	std::list<std::string> * o_queries)
{
	// Get task exec parameters:
	m_command   = i_exec->getCommand();
	m_username  = i_exec->getUserName();
	m_jobname   = i_exec->getJobName();
	m_blockname = i_exec->getBlockName();
	m_service   = i_exec->getServiceType();
	m_capacity  = i_exec->getCapacity();

	m_starts_count = i_progress->starts_count;
	m_errors_count = i_progress->errors_count;
	m_time_start   = i_progress->time_start;
	m_time_done    = i_progress->time_done;
	m_error      = ( i_progress->state & AFJOB::STATE_ERROR_MASK ) ? 1 : 0;

	m_hostname = i_render->getName();

	m_description = i_job->getDescription();
	m_folder      = i_job->getFolder();
	m_annotation  = i_job->getAnnotation();

	// Skip not started exec:
	if( m_time_start == 0 ) return;
	// Skip exec with no running time:
	if( m_time_start == m_time_done ) return;
	// Set done time to current time, if exec was not done:
	if( m_time_done < m_time_start ) m_time_done = time( NULL);

	// Insert row:
	v_dbInsert( o_queries);
}
