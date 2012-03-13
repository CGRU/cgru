#pragma once

#include "../../include/aftypes.h"

#include "../msg.h"
#include "../address.h"

#include "msgclassuserhost.h"

#include <QtCore/QString>

namespace af
{

class MCListenAddress : public MsgClassUserHost
{
public:
   MCListenAddress( uint8_t Flags, const Address * address, int JobId = 0, int BlockNum = 0, int TaskNum = 0, int Number = 0);
   MCListenAddress( Msg * msg);
   ~MCListenAddress();

   void stdOut( bool full = false ) const;

   inline uint8_t getFlags()   const { return flags;}
   inline bool    toListen()   const { return flags & TOLISTEN;   }
   inline bool    justTask()   const { return flags & JUSTTASK;   }
   inline bool    fromRender() const { return flags & FROMRENDER; }

   inline int getJobId()    const { return jobid;    }
   inline int getNumBlock() const { return blocknum; }
   inline int getNumTask()  const { return tasknum;  }
   inline int getNumber()   const { return number;   }

   inline const Address * getAddress() const { return address_ptr; }
   inline void setIP( const Address * other) { address_ptr->setIP( other);}

public:
   static const uint8_t TOLISTEN   = 1<<0;
   static const uint8_t JUSTTASK   = 1<<1;
   static const uint8_t FROMRENDER = 1<<2;

private:
   int32_t jobid;
   int32_t blocknum;
   int32_t tasknum;
   int32_t number;
   uint8_t  flags;

   Address * address_ptr;

   void readwrite( Msg * msg);
};
}
