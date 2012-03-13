#include "mclistenaddress.h"

#include <stdio.h>

#include "../name_af.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../../include/macrooutput.h"

using namespace af;

MCListenAddress::MCListenAddress( uint8_t Flags, const Address * address, int JobId, int BlockNum, int TaskNum, int Number):
   jobid(      JobId       ),
   blocknum(   BlockNum    ),
   tasknum(    TaskNum     ),
   number(     Number      ),
   flags(Flags)
{
   address_ptr = new Address( address);
}

MCListenAddress::MCListenAddress( Msg * msg)
{
   read( msg);
}

MCListenAddress::~MCListenAddress()
{
   if( address_ptr != NULL) delete address_ptr;
}

void MCListenAddress::readwrite( Msg * msg)
{
   MsgClassUserHost::readwrite( msg);

   rw_int32_t( jobid,        msg);
   rw_int32_t( blocknum,     msg);
   rw_int32_t( tasknum,      msg);
   rw_int32_t( number,       msg);
   rw_uint8_t( flags,        msg);

   if( msg->isWriting() ) address_ptr->write( msg);
   else address_ptr = new Address( msg);
}

void MCListenAddress::stdOut(bool full) const
{
   printf("Listen(j%d,b%d,t%d,n%d): %s-%s-%s: ", jobid, blocknum, tasknum, number,
      (flags&TOLISTEN)?"ON":"OFF",(flags&JUSTTASK)?"TASK":"JOB",(flags&FROMRENDER)?"RENDER":"CLIENT");
   address_ptr->stdOut();
   printf(" - ");
   MsgClassUserHost::stdOut();
   printf("\n");
}
