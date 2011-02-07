#pragma once

#include <QtCore/QString>

#include "afqueue.h"
#include "afqueueitem.h"

class LogData: public AfQueueItem
{
public:
   LogData( const QString & str);
   void output();
private:
   QString text;
};

class LogQueue : public AfQueue
{
public:
   LogQueue ( const QString & QueueName);
   virtual ~LogQueue();

/// Push queries to queue back.
   inline bool pushLog( const QString & log) { return push( new LogData(log));}

protected:
   void processItem( AfQueueItem* item) const;
};
