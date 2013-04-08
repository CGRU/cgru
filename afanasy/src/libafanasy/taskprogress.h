#pragma once

#include "../include/afjob.h"

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

   virtual void v_generateInfoStream( std::ostringstream & stream, bool full = false) const; ///< Generate information string.

   virtual int calcWeight() const;

   uint32_t state;         ///< state per block per task.
   int8_t   percent;       ///< percent per block per task.
   int64_t  frame;         ///< frame per block per task.
   int8_t   percentframe;  ///< frame percent per block per task.
   int32_t  starts_count;  ///< number of starts per block per task.
   int32_t  errors_count;  ///< Number of times task finished with errors .
   int64_t  time_start;    ///< start time.
   int64_t  time_done;     ///< Task finish time ( or last update time if still running ).

	inline void setSolved() { state |= AFJOB::STATE_SOLVED_MASK; }
	inline void setNotSolved() { state &= (~AFJOB::STATE_SOLVED_MASK); }
	inline bool isSolved() const { return state & AFJOB::STATE_SOLVED_MASK; }
	inline bool isNotSolved() const { return false == (state & AFJOB::STATE_SOLVED_MASK); }

   std::string hostname;           ///< Host, last event occurs where.
	std::string activity;			///< Task activity that was parsed.

   void v_readwrite( Msg * msg); ///< Read or write progress in buffer.

	void jsonWrite( std::ostringstream & o_str) const;
};
}
