#pragma once

#include "msgclass.h"

#include "../taskprogress.h"

namespace af
{

class MCTasksProgress : public MsgClass
{
public:
   MCTasksProgress( int JobId);
   MCTasksProgress( Msg * msg);
   ~MCTasksProgress();

   void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

   void add( int block, int task, TaskProgress * tp);

   inline int getJobId() const { return jobid;      }
   inline size_t getCount() const { return tasks.size();}

   inline const std::list<int32_t> * getBlocks() const { return &blocks; }
   inline const std::list<int32_t> * getTasks()  const { return &tasks;  }
   inline std::list<TaskProgress*> * getTasksRun() { return &tasksprogress; }

private:
   bool clientside;
   int32_t jobid;

   std::list<int32_t> blocks;
   std::list<int32_t> tasks;

   std::list<TaskProgress*> tasksprogress;

private:
   void v_readwrite( Msg * msg);
};
}
