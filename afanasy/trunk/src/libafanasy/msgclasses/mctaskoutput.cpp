#include "mctaskoutput.h"

#include <stdio.h>

#include "../name_af.h"
#include "../msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../../include/macrooutput.h"

using namespace af;

MCTaskOutput::MCTaskOutput(   const QString & RenderName,
                              int JobId, int BlockNum, int TaskNum,
                              int DataSize, char * Data):
   jobid(      JobId       ),
   blocknum(   BlockNum    ),
   tasknum(    TaskNum     ),
   datasize(   DataSize    ),
   rendername( RenderName  ),
   data( NULL )
{
   if( allocateData()) memcpy( data, Data, datasize);
}

MCTaskOutput::MCTaskOutput( Msg * msg)
{
   read( msg);
}

MCTaskOutput::~MCTaskOutput()
{
   if( data ) delete [] data;
}

bool MCTaskOutput::allocateData()
{
   if( datasize == 0) return false;
   data = new char[datasize];
   if( data == NULL )
   {
      AFERROR("MCTaskOutput::allocateData: Can't allocate %d bytes for data.\n");
      datasize = 0;
      return false;
   }
   return true;
}

void MCTaskOutput::readwrite( Msg * msg)
{
   rw_int32_t( jobid,        msg);
   rw_int32_t( blocknum,     msg);
   rw_int32_t( tasknum,      msg);
   rw_int32_t( datasize,     msg);

   rw_QString( rendername,   msg);

   if( msg->isReading()) allocateData();
   if( datasize > 0 ) rw_data( data, msg, datasize);
}

#ifdef WINNT
#include <io.h>
#endif
void MCTaskOutput::stdOut(bool full) const
{
   printf("Job id = %d, numblock = %d, numtask = %d, data size=%d", jobid, blocknum, tasknum, datasize);
   if( full )
   {
      printf("\n");
      int integer = 0;
#ifdef WINNT
      integer = ::_write( 1, data, datasize);
#else
      integer = ::write( 1, data, datasize);
#endif
   }
   printf("\n");
}
