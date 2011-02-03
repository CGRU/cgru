#pragma once

class AfQueue;

/// Queue item
class AfQueueItem
{
public:

/// Constructor.
   AfQueueItem();

   virtual ~AfQueueItem();

   friend class AfQueue;

private:
   mutable AfQueueItem * next_ptr;  ///< Next item pointer.
};
