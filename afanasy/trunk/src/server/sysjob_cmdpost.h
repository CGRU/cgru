#pragma once

#include "sysjob.h"

/// System job block:
class SysBlock_CmdPost : public SysBlock
{
public:
   SysBlock_CmdPost( af::Job * blockJob, af::BlockData * blockData, af::JobProgress * progress, std::list<std::string> * log);
   ~SysBlock_CmdPost();

};

/// System job block data:
class SysBlockData_CmdPost : public SysBlockData
{
public:
   SysBlockData_CmdPost( int BlockNum, int JobId);
   ~SysBlockData_CmdPost();

};
