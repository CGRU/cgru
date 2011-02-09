#include "logqueue.h"

#include <iostream>

#include "../libafanasy/name_af.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

LogData::LogData( const std::string & str)
{
   text = af::time2str();
   text += ": ";
   text += str;
}

void LogData::output()
{
   std::cout << text << std::endl;
   std::cout.flush();
}

LogQueue::LogQueue( const std::string & QueueName):  AfQueue( QueueName) {}

LogQueue::~LogQueue() {}

void LogQueue::processItem( AfQueueItem* item) const
{
   LogData * data = (LogData*)item;
   data->output();
}
