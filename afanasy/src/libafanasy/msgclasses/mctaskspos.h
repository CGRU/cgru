#pragma once

#include <stdlib.h>
#include <vector>

#include "msgclassuserhost.h"

namespace af
{

class MCTasksPos : public MsgClassUserHost
{
public:
   MCTasksPos( int job_id, const std::string & Message);
   MCTasksPos( Msg * msg);
   ~MCTasksPos();

   bool addBlock( int numblock);
   bool addTask(  int numblock, int numtask);

   inline bool hasTasks() const { return has_tasks;}

   void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

   inline int    getJobId() const { return jobid;          }
   inline size_t getCount() const { return numbloks.size();}
   inline const std::string & getMessage() const { return message; }

   inline int getNumBlock( const unsigned pos) const
      { if( pos < numbloks.size()) return numbloks[pos]; else return -1; }
   inline int getNumTask(  const unsigned pos) const
      { if( has_tasks && (pos < numbloks.size())) return numtasks[pos]; else return -1; }

private:
   int32_t jobid;
   bool     has_tasks;
   std::vector<int32_t> numbloks;
   std::vector<int32_t> numtasks;
   std::string message;

   void v_readwrite( Msg * msg);
};
}
