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
   clientid        ( ClientId),

   numjob          ( NumJob),
   numblock        ( NumBlock),
   numtask         ( NumTask),
   number          ( Number),

   status          ( UpStatus),
   percent         ( UpPercent),
   frame           ( UpFrame),
   percent_frame   ( UpPercentFrame),
   datalen         ( DataLen ),
   data            ( Data )
{
}

MCTaskUp::MCTaskUp( Msg * msg):
   data ( NULL )
{
   read( msg);
}

MCTaskUp::~MCTaskUp()
{
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
         AFERROR("MCTaskUp::readwrite: data == NULL.\n");
         return;
      }
//      memcpy( buffer+ws, data, datalen);
      rw_data( data, msg, datalen);
   }
   else
   {
//printf("MCTaskUp::readwrite: task with output data recieved: datalen = %d\n", datalen);
      data = new char[datalen];
      if( data == NULL )
      {
         AFERROR("MCTaskUp::readwrite: Can't allocate memory for data.\n");
         datalen = 0;
         return;
      }
//      memcpy( data, buffer+ws, datalen);
      rw_data( data, msg, datalen);
//printf("MCTaskUp::readwrite: data copied.\n");
   }
}

void MCTaskUp::stdOut( bool full ) const
{
   printf("TaskUp: client=%d, job=%d, block=%d, task=%d, number=%d, datalen=%d, status=%d, percent=%d\n",
                    clientid, numjob, numblock, numtask, number, datalen, status, percent);

   if( full && datalen && data) printf("data:\n%s\n", data);
}
