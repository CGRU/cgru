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

MCTaskPos::MCTaskPos( const MCTaskUp & i_tup):
	jobid(    i_tup.getNumJob()   ),
	blocknum( i_tup.getNumBlock() ),
	tasknum(  i_tup.getNumTask()  ),
	number(   i_tup.getNumber()   )
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

void MCTaskPos::jsonWrite( std::ostringstream & o_str) const
{
	o_str << "{\"job\":"    << jobid;
	o_str << ",\"block\":"  << blocknum;
	o_str << ",\"task\":"   << tasknum;
	o_str << ",\"number\":" << number << "}";
}

bool MCTaskPos::isEqual( const MCTaskPos & i_other) const
{
	if( ( jobid    == i_other.jobid    ) &&
		( blocknum == i_other.blocknum ) &&
		( tasknum  == i_other.tasknum  ) &&
		( number   == i_other.number   ))
		return true;

	return false;
}

bool MCTaskPos::isSameTask( const MCTaskPos & i_other) const
{
	if( ( jobid    == i_other.jobid    ) &&
		( blocknum == i_other.blocknum ) &&
		( tasknum  == i_other.tasknum  ))
		return true;

	return false;
}

void MCTaskPos::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
	if( full )
	{
		stream << "MCTaskPos: Job id = " << jobid << ", numblock = " << blocknum << ", numtask = " << tasknum;
		if( number ) stream << ", number = " << number;
	}
	else
	{
		stream << "TP: j" << jobid << " b" << blocknum << " t" << tasknum;
		if( number ) stream << "(" << number << ")";
	}
}
