#pragma once

#include <pthread.h>

#include "../libafanasy/name_af.h"

#include "afthread.h"

/// Main Afanasy core thread.
class ThreadRun : public ThreadAf
{
public:
   ThreadRun( const ThreadPointers *ptrs, int seconds = 1);
   ~ThreadRun();

   void run();             ///< Thread loop function. Return \c false on fail.

   void msgCase( MsgAf *msg);   ///< Message type switch.

private:
};
