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
   JobProgress( Job * job, bool doConstruct = true);

/// Construct job progress from given data buffer.
   JobProgress( Msg * msg);

   virtual ~JobProgress();

   virtual void v_generateInfoStream( std::ostringstream & stream, bool full = false) const; ///< Print job progress information.

   inline void setJobId( int id ) { m_job_id = id;   } ///< Set job id.
   inline int  getJobId() const   { return m_job_id; } ///< Get job id.

   inline int getBlocksNum() const { return m_blocks_num;} ///< Get blocks number.

/// Get tasks number in \c b block.
   inline int getTasksNum( int b) const
      { if( b < m_blocks_num ) return tasksnum[b]; else return -1;}
   void appendTasks(int block, int numtasks);

   virtual int calcWeight() const;                   ///< Calculate and return memory size.

	void jsonWrite( std::ostringstream & o_str) const;

public:
   TaskProgress  ***tp;

protected:
   bool construct( Job * job);               ///< Construct progress blocks and tasks data.
   bool initBlocks();                        ///< Allocate memory for blocks data.
   bool initTasks( int block, int numtasks); ///< Allocate memory for tasks  data.

protected:
   int32_t m_blocks_num;           ///< Block number.
   int32_t *tasksnum;           ///< Block tasks number per block.

private:
	void initProperties();
	virtual TaskProgress * newTaskProgress() const;

private:
   int32_t m_job_id;               ///< Job id.

private:
   void v_readwrite( Msg * msg); ///< Read or write progress in buffer.
};
}
