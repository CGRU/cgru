#include "jobprogress.h"

#include "msg.h"
#include "job.h"
#include "blockdata.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

JobProgress::JobProgress( Job * job, bool doConstruct):
   m_job_id( job->getId())
{
	initProperties();
	if( doConstruct ) construct( job);
}

bool JobProgress::construct( Job * job)
{
   m_blocks_num = job->getBlocksNum();
   if( m_blocks_num < 1)
   {
      AFERRAR("JobProgress::JobProgress: Invalid number if blocks = %d (job name: '%s')", m_blocks_num, job->getName().c_str())
      return false;
   }

   if( initBlocks() == false)
   {
      AFERROR("JobProgress::JobProgress: Blocks initalization failed.")
      return false;
   }

   for( int b = 0; b < m_blocks_num; b++)
   {
      const af::BlockData * block = job->getBlock( b);
      tasksnum[b] = block->getTasksNum();
      if( tasksnum[b] < 1)
      {
         AFERRAR("JobProgress::JobProgress: Invalid number of tasks = %d (m_job_id=%d,block=%d)", tasksnum[b], job->getId(), b)
         return false;
      }

      if( initTasks( b, tasksnum[b]) == false)
      {
         AFERRAR("JobProgress::JobProgress: Tasks initalization failed ( block=%d, tasks number=%d).", b, tasksnum[b])
         return false;
      }
   }

   return true;
}

bool JobProgress::reconstruct( Job * job)
{
   int32_t old_blocks_num = m_blocks_num;
   int32_t new_blocks_num = job->getBlocksNum();
   if( new_blocks_num == old_blocks_num)
	   return true;

   if( new_blocks_num < 1)
   {
      AFERRAR("JobProgress::JobProgress: Invalid number if blocks = %d (job name: '%s')", m_blocks_num, job->getName().c_str())
      return false;
   }

   appendBlocks( new_blocks_num - old_blocks_num);

   for( int b = old_blocks_num; b < m_blocks_num; b++)
   {
      const af::BlockData * block = job->getBlock( b);
      tasksnum[b] = block->getTasksNum();
      if( tasksnum[b] < 1)
      {
         AFERRAR("JobProgress::JobProgress: Invalid number of tasks = %d (m_job_id=%d,block=%d)", tasksnum[b], job->getId(), b)
         return false;
      }

      if( initTasks( b, tasksnum[b]) == false)
      {
         AFERRAR("JobProgress::JobProgress: Tasks initalization failed ( block=%d, tasks number=%d).", b, tasksnum[b])
         return false;
      }
   }

   return true;
}

JobProgress::JobProgress( Msg * msg)
{
	initProperties();
	read( msg);
}

void JobProgress::initProperties()
{
	m_job_id = 0;
	tasksnum = NULL;
	tp = NULL;
	m_blocks_num = 0;
}

bool JobProgress::initBlocks()
{
   if( m_blocks_num == 0)
   {
      AFERROR("JobProgress::initialize: m_blocks_num == 0\n");
      return false;
   }

   tasksnum = new int32_t        [ m_blocks_num];
   tp       = new TaskProgress **[ m_blocks_num]();

   return true;
}

void JobProgress::appendBlocks(int numblocks)
{
	int32_t old_blocks_num = m_blocks_num;
	m_blocks_num += numblocks;

	int32_t * old_tasksnum = tasksnum;
	tasksnum = new int32_t[m_blocks_num];
	for (int b = 0; b < m_blocks_num; b++)
		tasksnum[b] = b < old_blocks_num ? old_tasksnum[b] : 0;
	if( old_tasksnum != NULL) delete [] old_tasksnum;

	TaskProgress ***old_tp = tp;
	tp = new TaskProgress **[ m_blocks_num]();
	for (int b = 0; b < m_blocks_num; b++)
		tp[b] = b < old_blocks_num ? old_tp[b] : 0;
	if( old_tp != NULL) delete [] old_tp;
}

bool JobProgress::initTasks( int block, int numtasks)
{
   if( numtasks == 0)
   {
      AFERROR("JobProgress::initTasks: numtasks == 0\n");
      return false;
   }
   tp[block] = new TaskProgress*[ numtasks];
   for( int t = 0; t < tasksnum[block]; t++)
      tp[block][t] = newTaskProgress();

   return true;
}

void JobProgress::appendTasks(int block, int numtasks)
{
	int32_t old_tasksnum = tasksnum[block];
	TaskProgress **old_tp = tp[block];

	tasksnum[block] += numtasks;
	tp[block] = new TaskProgress*[ tasksnum[block]];

	for( int t = 0; t < tasksnum[block]; t++)
		tp[block][t] = t < old_tasksnum ? old_tp[t] : newTaskProgress();

	if( old_tp != NULL) delete [] old_tp;
}

TaskProgress * JobProgress::newTaskProgress() const
{
   return new TaskProgress;
}

JobProgress::~JobProgress()
{
AFINFA("JobProgress::~JobProgress: Job Id = %d", m_job_id)
   if( tp != NULL )
   {
      AFINFO("JobProgress::~JobProgress: Deleting tasks running information.")
      for( int b = 0; b < m_blocks_num; b++)
      {
         if( tp[b] != NULL )
         {
            for( int t = 0; t < tasksnum[b]; t++)
            {
               if( tp[b][t] != NULL) delete tp[b][t];
            }
            delete [] tp[b];
         }
      }
      delete [] tp;
   }
   if( tasksnum != NULL ) delete [] tasksnum;
}

void JobProgress::v_readwrite( Msg * msg)
{
   rw_int32_t( m_job_id,     msg);
   rw_int32_t( m_blocks_num, msg);

   if( msg->isReading() )
   {
      if( initBlocks() == false)
      {
         AFERROR("JobProgress::readwrite: blocks initalization failed.")
         return;
      }
   }

   for( int b = 0; b < m_blocks_num; b++)
   {
      rw_int32_t( tasksnum[b],        msg);
      if( tasksnum[b] == 0 ) continue;
      if( msg->isReading() )
      {
         if( initTasks( b, tasksnum[b]) == false)
         {
            AFERRAR("JobProgress::readwrite: tasks initalization failed ( block=%d, tasks number=%d).", b, tasksnum[b])
            return;
         }
      }
      for( int t = 0; t < tasksnum[b]; t++)
      {
         tp[b][t]->v_readwrite( msg);
      }
   }
}


void JobProgress::jsonWrite( std::ostringstream & o_str) const
{
//	o_str << "void JobProgress::jsonWrite( std::ostringstream & o_str) const";
	o_str << "{\"job_progress\":{";
	o_str << "\"id\":" << m_job_id << ",\n";
	o_str << "\"progress\":[\n";
	for( int b = 0; b < m_blocks_num; b++)
	{
		if( b > 0 )
			o_str << ",\n";
		o_str << "[";
		for( int t = 0; t < tasksnum[b]; t++)
		{
			if( t > 0 )
				o_str << ",\n";
			tp[b][t]->jsonWrite( o_str);
		}
		o_str << "]";
	}
	o_str << "\n]}}";
}

int JobProgress::calcWeight() const
{
   int weight  = sizeof(JobProgress);
   for( int b = 0; b < m_blocks_num; b++)
   {
      weight += sizeof(*tasksnum);
      weight += tasksnum[b] * sizeof(**tp);
      for( int t = 0; t < tasksnum[b]; t++)
         weight += tp[b][t]->calcWeight();
   }
   return weight;
}

void JobProgress::v_generateInfoStream( std::ostringstream & stream, bool full ) const
{
    for( int b = 0; b < m_blocks_num; b++)
    {
        if( b > 0 ) stream << std::endl;
        stream << "Block #"<< b;
        for( int t = 0; t < tasksnum[b]; t++)
        {
            stream << std::endl;
            tp[b][t]->v_generateInfoStream( stream);
        }
    }
}
