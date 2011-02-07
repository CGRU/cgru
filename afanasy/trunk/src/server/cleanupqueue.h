#pragma once

#include "afqueue.h"
#include "afqueueitem.h"

class JobAf;

class CleanUpData: public AfQueueItem
{
public:
   CleanUpData( const JobAf * job);
   void doCleanUp();
private:
   char tasksoutdir[512];
};

class CleanUpQueue : public AfQueue
{
public:
   CleanUpQueue( const QString & QueueName);
   virtual ~CleanUpQueue();

/// Push queries to queue back.
   inline bool pushJob( const JobAf * job) { return push( new CleanUpData(job));}

protected:
   void processItem( AfQueueItem* item) const;
};
