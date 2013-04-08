#include "mctaskpos.h"

#include "../name_af.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../../include/macrooutput.h"

using namespace af;

MCTaskPos::MCTaskPos( int job_id, int block_num, int task_num, int Number):
   jobid(    job_id    ),
   blocknum( block_num ),
   tasknum(  task_num  ),
   number(   Number    )
{
}

MCTaskPos::MCTaskPos( Msg * msg)
{
   read( msg);
}

MCTaskPos::~MCTaskPos()
{
}

void MCTaskPos::v_readwrite( Msg * msg)
{
   rw_int32_t( jobid,    msg);
   rw_int32_t( blocknum, msg);
   rw_int32_t( tasknum,  msg);
   rw_int32_t( number,   msg);
}

void MCTaskPos::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
   stream << "Job id = " << jobid << ", numblock = " << blocknum << ", numtask = " << tasknum;
}
