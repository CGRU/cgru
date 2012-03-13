#ifndef MCTASKOUTPUT_H
#define MCTASKOUTPUT_H

#include <aftypes.h>

#include "msgclass.h"

#include <QtCore/QString>

namespace af
{

class MCTaskOutput : public MsgClass
{
public:
   MCTaskOutput(  const QString & RenderName,
                  int JobId, int BlockNum, int TaskNum,
                  int DataSize, char * Data);
   MCTaskOutput( Msg * msg);
   ~MCTaskOutput();

   void stdOut( bool full = false ) const;

   inline const QString& getRenderName() const { return rendername; }
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
   QString rendername;
   char * data;

private:
   bool allocateData();
   void readwrite( Msg * msg);
};
}
#endif
