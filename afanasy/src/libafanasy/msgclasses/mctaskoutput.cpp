#include "mctaskoutput.h"

#include <stdio.h>
#include <string.h>

#include "../name_af.h"
#include "../msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../../include/macrooutput.h"

using namespace af;

MCTaskOutput::MCTaskOutput(   const std::string & RenderName,
                              int JobId, int BlockNum, int TaskNum,
                              int DataSize, const char * Data):
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

void MCTaskOutput::v_readwrite( Msg * msg)
{
   rw_int32_t( jobid,        msg);
   rw_int32_t( blocknum,     msg);
   rw_int32_t( tasknum,      msg);
   rw_int32_t( datasize,     msg);

   rw_String ( rendername,   msg);

   if( msg->isReading()) allocateData();
   if( datasize > 0 ) rw_data( data, msg, datasize);
}

void MCTaskOutput::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
   stream << "Job id = " << jobid << ", numblock = " << blocknum << ", numtask = " << tasknum << ", data size= " << datasize;
   if( full )
   {
      stream << std::endl;
      stream << std::string( data, datasize);
   }
}
