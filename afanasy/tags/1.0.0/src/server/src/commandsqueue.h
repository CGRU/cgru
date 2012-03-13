#pragma once

#include <QtCore/QString>

#include "afqueue.h"
#include "afqueueitem.h"

class Command: public QString, public AfQueueItem
{
public:
   Command( const QString & cmd);
};

/// Simple FIFO commands queue
class CommandsQueue : public AfQueue
{
public:
   CommandsQueue( const QString & QueueName);
   ~CommandsQueue();

/// Push queries to queue back.
   inline bool pushCommand( const QString & cmd) { return push( new Command(cmd));}

protected:
   void processItem( AfQueueItem* item) const;
};
