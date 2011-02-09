#include "jobprogress.h"

#include "msg.h"
#include "job.h"
#include "blockdata.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

JobProgress::JobProgress( Job * job):
   jobid( job->getId())
{
}

JobProgress::JobProgress( Msg * msg)
{
   read( msg);
}

bool JobProgress::initBlocks()
{
   tasksnum    = NULL;
   tp          = NULL;

   if( blocksnum == 0)
   {
      AFERROR("JobProgress::initialize: blocksnum == 0\n");
      return false;
   }

   tasksnum = new int32_t        [ blocksnum];
   tp       = new TaskProgress **[ blocksnum];

   return true;
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

TaskProgress * JobProgress::newTaskProgress() const
{
   return new TaskProgress;
}

JobProgress::~JobProgress()
{
AFINFA("JobProgress::~JobProgress: Job Id = %d\n", jobid);
   if( tp != NULL )
   {
      AFINFO("JobProgress::~JobProgress: Deleting tasks running information.\n");
      for( int b = 0; b < blocksnum; b++)
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

void JobProgress::readwrite( Msg * msg)
{
   rw_int32_t( jobid,     msg);
   rw_int32_t( blocksnum, msg);

   if( msg->isReading() )
   {
      if( initBlocks() == false)
      {
         AFERROR("JobProgress::readwrite: blocks initalization failed.\n");
         return;
      }
   }

   for( int b = 0; b < blocksnum; b++)
   {
      rw_int32_t( tasksnum[b],        msg);
      if( tasksnum[b] == 0 ) continue;
      if( msg->isReading() )
      {
         if( initTasks( b, tasksnum[b]) == false)
         {
            AFERRAR("JobProgress::readwrite: tasks initalization failed ( block=%d, tasks number=%d).\n", b, tasksnum[b]);
            return;
         }
      }
      for( int t = 0; t < tasksnum[b]; t++)
      {
         tp[b][t]->readwrite( msg);
      }
   }
}

int JobProgress::calcWeight() const
{
   int weight  = sizeof(JobProgress);
   for( int b = 0; b < blocksnum; b++)
   {
      weight += sizeof(*tasksnum);
      weight += tasksnum[b] * sizeof(**tp);
      for( int t = 0; t < tasksnum[b]; t++)
         weight += tp[b][t]->calcWeight();
   }
   return weight;
}

void JobProgress::generateInfoStream( std::ostringstream & stream, bool full ) const
{
   for( int b = 0; b < blocksnum; b++)
   {
      stream << "Block #"<< b;
      for( int t = 0; t < tasksnum[b]; t++)
      {
         stream << std::endl;
         tp[b][t]->generateInfoStream( stream);
      }
   }
}
