#pragma once

#include "../libafanasy/afqueue.h"

class LogData: public af::AfQueueItem
{
public:
   LogData( const std::string & str, int flags = Info);
   void output();

   enum Flags
   {
      Info  = 0,
      Error = 1,
      Errno = 2
   };

private:
   std::string text;
};

class LogQueue : public af::AfQueue
{
public:
   LogQueue ( const std::string & QueueName);
   virtual ~LogQueue();

/// Push queries to queue back.
   inline bool pushLog( const std::string & log, int flags) { return push( new LogData(log, flags));}

protected:
   void processItem( af::AfQueueItem* item);
};
