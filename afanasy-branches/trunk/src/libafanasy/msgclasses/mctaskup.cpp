#include "mctaskup.h"

#include <stdio.h>
#include <memory.h>

#include "../msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../../include/macrooutput.h"

using namespace af;

MCTaskUp::MCTaskUp
(
   int ClientId,

   int NumJob,
   int NumBlock,
   int NumTask,
   int Number,

   int UpStatus,
   int UpPercent,
   int UpFrame,
   int UpPercentFrame,
   int DataLen,
   char * Data
):
   clientid       ( ClientId),

   numjob         ( NumJob),
   numblock       ( NumBlock),
   numtask        ( NumTask),
   number         ( Number),

   status         ( UpStatus),
   percent        ( UpPercent),
   frame          ( UpFrame),
   percent_frame  ( UpPercentFrame),
   datalen        ( DataLen ),
   data           ( Data ),
   deleteData     ( false) // Don not delete data on client side, as it is not copied
{
}

MCTaskUp::MCTaskUp( Msg * msg):
   data ( NULL),
   deleteData( true)       // Delete data on server side, as it was allocated and copied from incoming message
{
   read( msg);
}

MCTaskUp::~MCTaskUp()
{
   if( deleteData && data ) delete [] data;
}

void MCTaskUp::readwrite( Msg * msg)
{
   rw_int32_t( clientid,       msg);

   rw_int32_t( numjob,         msg);
   rw_int32_t( numblock,       msg);
   rw_int32_t( numtask,        msg);
   rw_int32_t( number,         msg);

   rw_int8_t ( status,         msg);
   rw_int8_t ( percent,        msg);
   rw_int32_t( frame,          msg);
   rw_int8_t ( percent_frame,  msg);
   rw_int32_t( datalen,        msg);

   if( datalen == 0 ) return;

   if( msg->isWriting() )
   {
      if( data == NULL )
      {
         AFERROR("MCTaskUp::readwrite: data == NULL.")
         return;
      }
      rw_data( data, msg, datalen);
   }
   else
   {
      data = new char[datalen];
      if( data == NULL )
      {
         AFERROR("MCTaskUp::readwrite: Can't allocate memory for data.")
         datalen = 0;
         return;
      }
      rw_data( data, msg, datalen);
   }
}

void MCTaskUp::generateInfoStream( std::ostringstream & stream, bool full) const
{
   stream << "TaskUp: client=" << clientid
         << ", job=" << numjob
         << ", block=" << numblock
         << ", task=" << numtask
         << ", number=" << number
         << ", datalen=" << datalen
         << ", status=" << status
         << ", percent=" << percent;

   if( full && datalen && data) stream << "\ndata:\n" << std::string( data, datalen) << std::endl;
}
