#include "sysjob_wol.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   BLOCK    /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

SysBlock_WOL::SysBlock_WOL( af::Job * blockJob, af::BlockData * blockData, af::JobProgress * progress, std::list<std::string> * log):
   SysBlock( blockJob, blockData, progress, log)
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

   name = "wake-on-lan";
   service = AFJOB::SYSJOB_BLOCKSERVICE;

   tasksmaxruntime = AFJOB::SYSJOB_TASKMAXRUNTIME;

/// Maximum number or errors on same host for block NOT to avoid host
   errors_avoidhost = AFJOB::SYSJOB_ERRORS_AVIODHOST;
/// Maximum number or errors on same host for task NOT to avoid host
   errors_tasksamehost = AFJOB::SYSJOB_ERRORS_TASKSAMEHOST;
/// Maximum number of errors in task to retry it automatically
   errors_retries = AFJOB::SYSJOB_ERRORS_RETRIES;
/// Time from last error to remove host from error list
   errors_forgivetime = AFJOB::SYSJOB_ERRORS_FORGIVETIME;
}

SysBlockData_WOL::~SysBlockData_WOL()
{
}
