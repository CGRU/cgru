#pragma once

#include <stdint.h>

#include "../libafanasy/af.h"
#include "../libafanasy/dlMutex.h"
#include "../libafanasy/msg.h"

/// Messages store structure.
struct MSGS
{
   int32_t    count;
   int32_t    size;
};

namespace af
{

/// Class to collect, store, calculate messages statistics and write and read it from buffer.
class MsgStat : public af::Af
{
public:
   MsgStat();
   ~MsgStat();

/// Collect new message to class.
   void put( int type, int size);

   void writeStat( af::Msg * msg = NULL); ///< Calculate and write statistics to buffer, return written size.
   void readStat(  af::Msg * msg);        ///< Read statistics from buffer.

/// Write statistics to \c stdout .
   void v_stdOut( int columns = -1, int sorting = -1) const;

/// Write statistics to \c stdout ( default function ).
   void v_stdOut( bool full = false) const;

private:

   static const int DTIME = 10;     ///< Minimal division time.
   static const int DIVISIONS = 10; ///< Number of divisions in store.
   static const int STORE = 5;      ///< Number of stores.


private:

   bool initialized;          ///< Whether statistics successfully initialized ( memory was allocated ).
   DlMutex m_mutex;             ///< Mutex locker.

   MSGS msgs_T[af::Msg::TLAST];                       ///< Initial store ( minimal time ).
   MSGS msgs_STD[STORE][af::Msg::TLAST][DIVISIONS];   ///< General store.
   MSGS msgs_ST[STORE][af::Msg::TLAST];               ///< Statistics store.
   int32_t msgsizemax_T[af::Msg::TLAST];              ///< Maximum message size store.

   uint32_t lasttime_S[STORE];      ///< Last time division in store was updated.
   int      lastmsgs_S[STORE];      ///< Last division in store was updated.

   void v_readwrite( af::Msg * msg);  ///< Read | write statistics store.
};

}
