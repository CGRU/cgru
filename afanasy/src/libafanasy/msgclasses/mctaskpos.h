#pragma once

#include "msgclass.h"

namespace af
{

class MCTaskPos : public MsgClass
{
public:
   MCTaskPos( int job_id, int block_num, int task_num, int Number = 0);
   MCTaskPos( Msg * msg);
   ~MCTaskPos();

   void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

   inline int getJobId()    const { return jobid;    }
   inline int getNumBlock() const { return blocknum; }
   inline int getNumTask()  const { return tasknum;  }
   inline int getNumber()   const { return number;   }

private:
   int32_t jobid;
   int32_t blocknum;
   int32_t tasknum;
   int32_t number;

   void v_readwrite( Msg * msg);
};
}
