#pragma once

#include <stdio.h>

#include "../include/afjob.h"

#include "af.h"
#include "name_af.h"
#include "taskprogress.h"

namespace af
{
/// Job progress information class.
class JobProgress : public Af
{
public:
/// Construct progress for given job.
   JobProgress( Job * job);

/// Construct job progress from given data buffer.
   JobProgress( Msg * msg);

   virtual ~JobProgress();

   void stdOut( bool full = false ) const; ///< Print job progress information.

   inline void setJobId( int id ) { jobid = id;   } ///< Set job id.
   inline int  getJobId() const   { return jobid; } ///< Get job id.

   inline int getBlocksNum() const { return blocksnum;} ///< Get blocks number.

/// Get tasks number in \c b block.
   inline int getTasksNum( int b) const
      { if( b < blocksnum ) return tasksnum[b]; else return -1;}

   virtual int calcWeight() const;                   ///< Calculate and return memory size.

public:
   TaskProgress  ***tp;

protected:
   bool initBlocks();                        ///< Allocate memory for blocks data.
   bool initTasks( int block, int numtasks); ///< Allocate memory for tasks  data.

protected:
   int32_t blocksnum;           ///< Block number.
   int32_t *tasksnum;           ///< Block tasks number per block.

private:
   virtual TaskProgress * newTaskProgress() const;

private:
   int32_t jobid;               ///< Job id.

private:
   void readwrite( Msg * msg); ///< Read or write progress in buffer.
};
}
