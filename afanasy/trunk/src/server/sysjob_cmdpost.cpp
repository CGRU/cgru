#include "sysjob_cmdpost.h"

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   BLOCK    /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

SysBlock_CmdPost::SysBlock_CmdPost( JobAf * blockJob, af::BlockData * blockData, af::JobProgress * progress, std::list<std::string> * log):
   SysBlock( blockJob, blockData, progress, log)
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

   name = "post_commands";
   service = af::Environment::getSysPostCmdService();

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

SysBlockData_CmdPost::~SysBlockData_CmdPost()
{
}
