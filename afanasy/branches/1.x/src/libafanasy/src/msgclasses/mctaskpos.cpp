#include "mctaskpos.h"

#include <stdio.h>

#include <name_af.h>

//#define AFOUTPUT
////#undef AFOUTPUT
#include <macrooutput.h>

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

void MCTaskPos::readwrite( Msg * msg)
{
   rw_int32_t( jobid,    msg);
   rw_int32_t( blocknum, msg);
   rw_int32_t( tasknum,  msg);
   rw_int32_t( number,   msg);
}

void MCTaskPos::stdOut(bool full) const
{
   printf("Job id = %d, numblock = %d, numtask = %d:\n", jobid, blocknum, tasknum);
}
