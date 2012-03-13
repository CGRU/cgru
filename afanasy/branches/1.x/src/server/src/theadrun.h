#pragma once

#include <pthread.h>

#include <name_af.h>

#include "afthead.h"

/// Main Afanasy core thread.
class TheadRun : public TheadAf
{
public:
   TheadRun( const TreadPointers *ptrs, int seconds = 1);
   ~TheadRun();

   void run();             ///< Thread loop function. Return \c false on fail.

   void msgCase( MsgAf *msg);   ///< Message type switch.

private:
};
