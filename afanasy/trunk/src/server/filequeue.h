#pragma once

#include "afqueue.h"
#include "filedata.h"

/// Simple FIFO filedata queue
class FileQueue : public AfQueue
{
public:
   FileQueue( const std::string & QueueName);
   virtual ~FileQueue();

/// Push filedata to queue back.
   inline bool pushFile( FileData* filedata) { return push( filedata);}

protected:
   void processItem( AfQueueItem* item) const;

private:
   void renameNext( const QString & filename, int number, int maxnumber) const;
};
