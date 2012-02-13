#pragma once

#include "../libafanasy/afqueue.h"

class JobAf;

class CleanUpData: public AfQueueItem
{
public:
   CleanUpData( const JobAf * job);
   void doCleanUp();
private:
   std::string tasksoutdir;
};

class CleanUpQueue : public AfQueue
{
public:
   CleanUpQueue( const std::string & QueueName);
   virtual ~CleanUpQueue();

/// Push queries to queue back.
   inline bool pushJob( const JobAf * job) { return push( new CleanUpData(job));}

protected:
   void processItem( AfQueueItem* item);
};
