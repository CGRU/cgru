#pragma once

#include <QtCore/QString>

#include "afqueue.h"
#include "afqueueitem.h"

class LogData: public AfQueueItem
{
public:
   LogData( const std::string & str);
   void output();
private:
   std::string text;
};

class LogQueue : public AfQueue
{
public:
   LogQueue ( const std::string & QueueName);
   virtual ~LogQueue();

/// Push queries to queue back.
   inline bool pushLog( const std::string & log) { return push( new LogData(log));}

protected:
   void processItem( AfQueueItem* item) const;
};
