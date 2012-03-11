#pragma once

#include "../libafanasy/afqueue.h"

class JobAf;

class CleanUpData: public af::AfQueueItem
{
public:
   CleanUpData( const JobAf * job);
   void doCleanUp();
private:
   std::string tasksoutdir;
};

class CleanUpQueue : public af::AfQueue
{
public:
   CleanUpQueue( const std::string & QueueName);
   virtual ~CleanUpQueue();

/// Push queries to queue back.
   inline bool pushJob( const JobAf * job) { return push( new CleanUpData(job));}

protected:
   void processItem( af::AfQueueItem* item);
};
