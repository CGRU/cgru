#include "sysjob_cmdpost.h"

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   BLOCK    /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

SysBlock_CmdPost::SysBlock_CmdPost( JobAf * blockJob, af::BlockData * blockData, af::JobProgress * progress):
   SysBlock( blockJob, blockData, progress)
{
AFINFO("SysBlock_CmdPost::SysBlock_CmdPost:")
}

SysBlock_CmdPost::~SysBlock_CmdPost()
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   BLOCK DATA   //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SysBlockData_CmdPost::SysBlockData_CmdPost( int BlockNum, int JobId):
   SysBlockData( BlockNum, JobId)
{
AFINFO("SysBlockData_CmdPost::SysBlockData_CmdPost:")

   m_name = "post_commands";
   m_service = af::Environment::getSysPostCmdService();

   m_tasks_max_run_time = AFJOB::SYSJOB_TASKMAXRUNTIME;

/// Maximum number or errors on same host for block NOT to avoid host
   m_errors_avoid_host = AFJOB::SYSJOB_ERRORS_AVIODHOST;
/// Maximum number or errors on same host for task NOT to avoid host
   m_errors_task_same_host = AFJOB::SYSJOB_ERRORS_TASKSAMEHOST;
/// Maximum number of errors in task to retry it automatically
   m_errors_retries = AFJOB::SYSJOB_ERRORS_RETRIES;
/// Time from last error to remove host from error list
   m_errors_forgive_time = AFJOB::SYSJOB_ERRORS_FORGIVETIME;
}

SysBlockData_CmdPost::~SysBlockData_CmdPost()
{
}
