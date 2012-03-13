#include "afqueueitem.h"

#include "afqueue.h"

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

//AfQueue* AfQueueItem::queue = NULL;

AfQueueItem::AfQueueItem():
   next_ptr( NULL)
{
}

AfQueueItem::~AfQueueItem()
{
}
/*
void AfQueueItem::enqueue()
{
   if( queue == NULL )
   {
      AFERROR("AfQueueItem::enqueue: enqueue == NULL\n");
      return;
   }
   queue->push( this);
   AFINFO("AfQueueItem::enqueue: Item pushed into queue.\n");
}
*/
