#pragma once

#include "sysjob.h"

/// System job block:
class SysBlock_Events : public SysBlock
{
public:
	SysBlock_Events( JobAf * blockJob, af::BlockData * blockData, af::JobProgress * progress);
	~SysBlock_Events();

};

/// System job block data:
class SysBlockData_Events : public SysBlockData
{
public:
	SysBlockData_Events( int BlockNum, int JobId);
	~SysBlockData_Events();
};
