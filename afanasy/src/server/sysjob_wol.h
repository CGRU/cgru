#pragma once

#include "sysjob.h"

/// System job block:
class SysBlock_WOL : public SysBlock
{
public:
   SysBlock_WOL( JobAf * blockJob, af::BlockData * blockData, af::JobProgress * progress);
   ~SysBlock_WOL();

};

/// System job block data:
class SysBlockData_WOL : public SysBlockData
{
public:
   SysBlockData_WOL( int BlockNum, int JobId);
   ~SysBlockData_WOL();

};
