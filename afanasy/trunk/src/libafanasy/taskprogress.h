#pragma once

#include "name_af.h"
#include "msg.h"

namespace af
{
class TaskProgress : public Af
{
public:
   TaskProgress();
   TaskProgress( Msg * msg);

   virtual ~TaskProgress();

   void stdOut( bool full = false ) const;

   virtual int calcWeight() const;

   uint32_t state;         ///< state per block per task.
   int8_t   percent;       ///< percent per block per task.
   int32_t  frame;         ///< frame per block per task.
   int8_t   percentframe;  ///< frame percent per block per task.
   int32_t  starts_count;  ///< number of starts per block per task.
   int32_t  errors_count;  ///< Number of times task finished with errors .
   uint32_t time_start;    ///< start time.
   uint32_t time_done;     ///< Task finish time ( or last update time if still running ).

   QString hostname;           ///< Host, last event occurs where.

//private:
   void readwrite( Msg * msg); ///< Read or write progress in buffer.
};
}
