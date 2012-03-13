#pragma once

class AfQueue;

/// Queue item
class AfQueueItem
{
public:

/// Constructor.
   AfQueueItem();

   virtual ~AfQueueItem();

//   virtual void enqueue(); ///< Push item into its queue
//   inline static void setQueue( AfQueue *Queue) { queue = Queue;} ///< Set queue.

   friend class AfQueue;

private:
   mutable AfQueueItem * next_ptr;  ///< Next item pointer.
//   static  AfQueue     * queue;     ///< Items queue.
};
