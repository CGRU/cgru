#pragma once

#include "msgclass.h"

namespace af
{

class MCTaskOutput : public MsgClass
{
public:
   MCTaskOutput(  const std::string & RenderName,
                  int JobId, int BlockNum, int TaskNum,
                  int DataSize, const char * Data);
   MCTaskOutput( Msg * msg);
   ~MCTaskOutput();

   void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

   inline const std::string & getRenderName() const { return rendername; }
   inline int getJobId()         const { return jobid;      }
   inline int getNumBlock()      const { return blocknum;   }
   inline int getNumTask()       const { return tasknum;    }
   inline int getDataSize()      const { return datasize;   }
   inline const char * getData() const { return data;       }

private:
   int32_t jobid;
   int32_t blocknum;
   int32_t tasknum;
   int32_t datasize;
   std::string rendername;
   char * data;

private:
   bool allocateData();
   void v_readwrite( Msg * msg);
};
}
