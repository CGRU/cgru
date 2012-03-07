#include "logqueue.h"

#include <errno.h>
#include <iostream>

#include "../libafanasy/name_af.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

LogData::LogData( const std::string & str, int flags)
{
   switch(flags)
   {
   case Error:
   case Errno:
      text += "ERROR ";
   }

   text += af::time2str();
   text += ": ";
   text += str;

   switch(flags)
   {
   case Errno:
      text += "\n";
      text += strerror( errno);
   }
}

void LogData::output()
{
   std::cout << text << std::endl;
   std::cout.flush();
}

LogQueue::LogQueue( const std::string & QueueName):
    AfQueue( QueueName, af::AfQueue::e_start_thread)
{}

LogQueue::~LogQueue() {}

void LogQueue::processItem( af::AfQueueItem* item)
{
   LogData * data = (LogData*)item;
   data->output();
   delete data;
}
