#include "commandsqueue.h"

#include "afcommon.h"

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

Command::Command( const QString & cmd):
   QString( cmd)
{
}

CommandsQueue::CommandsQueue( const QString & QueueName):
   AfQueue( QueueName)
{
}

CommandsQueue::~CommandsQueue()
{
}

void CommandsQueue::processItem( AfQueueItem* item) const
{
   AFCommon::executeCmd( *((Command*)item));
}
