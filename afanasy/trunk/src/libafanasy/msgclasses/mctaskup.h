#pragma once

#include "../../include/aftypes.h"

#include "../name_af.h"

#include "msgclass.h"

namespace af
{

class MCTaskUp : public MsgClass
{
public:
   MCTaskUp();
   MCTaskUp
      (
         int ClientId,

         int NumJob,
         int NumBlock,
         int NumTask,
         int Number,

         int UpStatus,
         int UpPercent      = -1,
         int UpFrame        = -1,
         int UpPercentFrame = -1,
         int DataLen        = 0,
         char * Data        = NULL
      );
   MCTaskUp( Msg * msg);
   ~MCTaskUp();

   inline int getClientId()      const { return clientid;      }

   inline int getNumJob()        const { return numjob;        }
   inline int getNumBlock()      const { return numblock;      }
   inline int getNumTask()       const { return numtask;       }
   inline int getNumber()        const { return number;        }

   inline int getStatus()        const { return status;        }
   inline int getPercent()       const { return percent;       }
   inline int getFrame()         const { return frame;         }
   inline int getPercentFrame()  const { return percent_frame; }
   inline int getDataLen()       const { return datalen;       }
   inline const char * getData() const { return data;          }

   void generateInfoStream( std::ostringstream & stream, bool full = false) const;

private:
   int32_t clientid;

   int32_t numjob;
   int32_t numblock;
   int32_t numtask;
   int32_t number;

   int8_t  status;
   int8_t  percent;
   int32_t frame;
   int8_t  percent_frame;
   int32_t datalen;

   char *data;

   bool deleteData;

private:
   void readwrite( Msg * msg);
};
}
