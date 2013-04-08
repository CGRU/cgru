#include "mclistenaddress.h"

#include <stdio.h>

#include "../environment.h"
#include "../name_af.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../../include/macrooutput.h"

using namespace af;

MCListenAddress::MCListenAddress( uint8_t Flags, const Address & lAddress, int JobId, int BlockNum, int TaskNum, int Number):
   jobid(      JobId       ),
   blocknum(   BlockNum    ),
   tasknum(    TaskNum     ),
   number(     Number      ),
   address(    lAddress    ),
   flags(Flags)
{
}

MCListenAddress::MCListenAddress( Msg * msg)
{
   read( msg);
}

MCListenAddress::~MCListenAddress()
{
}

void MCListenAddress::v_readwrite( Msg * msg)
{
   MsgClassUserHost::v_readwrite( msg);

   rw_int32_t( jobid,        msg);
   rw_int32_t( blocknum,     msg);
   rw_int32_t( tasknum,      msg);
   rw_int32_t( number,       msg);
   rw_uint8_t( flags,        msg);

   address.v_readwrite( msg);
}

void MCListenAddress::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
   stream << "Listen";
   stream << " j" << jobid << " b" << blocknum << " t" << tasknum << " n" << number;
   stream << ": " << ((flags&TOLISTEN)?"ON":"OFF") << "-" << ((flags&JUSTTASK)?"TASK":"JOB") << "-" << ((flags&FROMRENDER)?"RENDER":"CLIENT");
   stream << " ";
   address.v_generateInfoStream( stream, full);
   stream << " - ";
   MsgClassUserHost::v_generateInfoStream( stream, false);
}
