#include "logqueue.h"

#include "../libafanasy/name_af.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

LogData::LogData( const QString & str)
{
   text = str;
}

void LogData::output()
{
   af::printTime();
   printf("%s\n", text.toUtf8().data());
   fflush( stdout);
}

LogQueue::LogQueue( const QString & QueueName):  AfQueue( QueueName) {}

LogQueue::~LogQueue() {}

void LogQueue::processItem( AfQueueItem* item) const
{
   LogData * data = (LogData*)item;
   data->output();
}
