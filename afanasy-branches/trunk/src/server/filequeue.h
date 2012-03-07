#pragma once

#include "../libafanasy/afqueue.h"

#include "filedata.h"

/// Simple FIFO filedata queue
class FileQueue : public af::AfQueue
{
public:
   FileQueue( const std::string & QueueName);
   virtual ~FileQueue();

/// Push filedata to queue back.
   inline bool pushFile( FileData* filedata) { return push( filedata);}

protected:
   void processItem( af::AfQueueItem* item);

private:
   void renameNext( const std::string & filename, int number, int maxnumber) const;
};
