#include "sysjob_wol.h"

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   BLOCK    /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

SysBlock_WOL::SysBlock_WOL( JobAf * blockJob, af::BlockData * blockData, af::JobProgress * progress):
   SysBlock( blockJob, blockData, progress)
{
AFINFO("SysBlock_WOL::SysBlock_WOL:")
}

SysBlock_WOL::~SysBlock_WOL()
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   BLOCK DATA   //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SysBlockData_WOL::SysBlockData_WOL( int BlockNum, int JobId):
   SysBlockData( BlockNum, JobId)
{
AFINFA("SysBlockData_WOL::SysBlockData_WOL: BlockNum=%d, JobId=%d", BlockNum, JobId)

   m_name = "wake-on-lan";
   m_service = af::Environment::getSysWolService();

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

SysBlockData_WOL::~SysBlockData_WOL()
{
}
