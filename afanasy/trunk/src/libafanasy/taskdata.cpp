#include "taskdata.h"

#include "msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

TaskData::TaskData()
{
}

TaskData::TaskData( Msg * msg)
{
   read( msg);
}

TaskData::~TaskData()
{
}

void TaskData::readwrite( Msg * msg)
{
//printf("TaskData::readwrite:\n");
   static bool name_only = false;
   rw_bool(    name_only,  msg);
   rw_QString( name,       msg);
   if( name_only) return;

   rw_QString( command,    msg);
   rw_QString( files,      msg);
   rw_QString( dependmask, msg);
   rw_QString( customdata, msg);
}

int TaskData::calcWeight() const
{
   int weight = sizeof(TaskData);

   weight += weigh(name);
   weight += weigh(command);
   weight += weigh(files);
   weight += weigh(dependmask);
   weight += weigh(customdata);

   return weight;
}

void TaskData::stdOut( bool full) const
{
   if( full)
   {
      printf("Name = '%s'\n", name.toUtf8().data());
      printf("Command = '%s'\n", command.toUtf8().data());
      printf("Files = '%s'\n", files.toUtf8().data());
      printf("Dependences = '%s'\n", dependmask.toUtf8().data());
      printf("Custom Data = '%s'\n", customdata.toUtf8().data());
      printf("Memory: %d bytes\n", calcWeight());
   }
   else
   {
      printf("N'%s' C'%s'", name.toUtf8().data(), command.toUtf8().data());
      if( false == files.isEmpty()     ) printf(" F'%s'", files.toUtf8().data());
      if( false == dependmask.isEmpty()) printf(" D'%s'", dependmask.toUtf8().data());
      if( false == customdata.isEmpty()      ) printf(" cd'%s'", customdata.toUtf8().data());
      printf(" %d bytes\n", calcWeight());
   }
}
