#include "mctasksprogress.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../../include/macrooutput.h"

using namespace af;

MCTasksProgress::MCTasksProgress( int JobId):
   clientside( false),
   jobid( JobId)
{
}

MCTasksProgress::MCTasksProgress( Msg * msg):
   clientside( true)
{
   read( msg);
}

MCTasksProgress::~MCTasksProgress()
{
   if( clientside)
   {
      std::list<TaskProgress*>::iterator trIt = tasksprogress.begin();
      while( trIt != tasksprogress.end()) delete *(trIt++);
   }
}

void MCTasksProgress::readwrite( Msg * msg)
{
   rw_int32_t    ( jobid        , msg );
   rw_Int32_List ( blocks       , msg );
   rw_Int32_List ( tasks        , msg );

   if( msg->isWriting())
   {
      std::list<TaskProgress*>::iterator trIt = tasksprogress.begin();
      while( trIt != tasksprogress.end())
      {
         (*trIt)->readwrite( msg);
         trIt++;
      }
   }
   else
   {
      int count = int(tasks.size());
      for( int i = 0; i < count; i++)
         tasksprogress.push_back( new TaskProgress( msg));
   }
}

void MCTasksProgress::add( int block, int task, TaskProgress * tp)
{
   int count = int( tasks.size());

   std::list<int32_t>::iterator bIt = blocks.begin();
   std::list<int32_t>::iterator tIt =  tasks.begin();

   for( int i = 0; i < count; i++) if((block == *(bIt++)) && (task == *(tIt++))) return;

   blocks.push_back(    block    );
   tasks.push_back(     task     );
   tasksprogress.push_back(  tp  );

//printf("MCTasksProgress::add (new): "); stdOut();
}

void MCTasksProgress::stdOut( bool full ) const
{
   int count = int( tasks.size());

   std::list<int32_t>::const_iterator bIt = blocks.begin();
   std::list<int32_t>::const_iterator tIt =  tasks.begin();

   std::list<TaskProgress*>::const_iterator trIt = tasksprogress.begin();

   printf("Job id = %d\n", jobid);
   for( int i = 0; i < count; i++)
   {
      printf("#[b%d,t%d]:", *(bIt++), *(tIt++));
      (*(trIt++))->stdOut( full);
   }
}
