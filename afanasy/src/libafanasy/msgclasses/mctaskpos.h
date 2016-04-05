#pragma once

#include "msgclass.h"
#include "mctaskup.h"

namespace af
{

class MCTaskPos : public MsgClass
{
public:
	MCTaskPos( int job_id = 0, int block_num = 0, int task_num = 0, int Number = 0);
	MCTaskPos( const MCTaskUp & i_tup);
   MCTaskPos( Msg * msg);
   ~MCTaskPos();

	bool equal( const MCTaskPos & i_other) const;

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
public:
   void v_readwrite( Msg * msg);
};
}
