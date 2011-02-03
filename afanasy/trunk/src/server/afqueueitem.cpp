#include "afqueueitem.h"

#include "afqueue.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

AfQueueItem::AfQueueItem():
   next_ptr( NULL)
{
}

AfQueueItem::~AfQueueItem()
{
}
