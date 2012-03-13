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
   rw_bool(    name_only,    msg);
   rw_QString( name,         msg);
   if( name_only) return;

   rw_QString( cmd,          msg);
   rw_QString( cmd_view,     msg);
   rw_QString( dependmask,   msg);
}

int TaskData::calcWeight() const
{
   int weight = sizeof(TaskData);

   weight += weigh(name);
   weight += weigh(cmd);
   weight += weigh(cmd_view);
   weight += weigh(dependmask);

   return weight;
}

void TaskData::stdOut( bool full) const
{
   if( full)
   {
      printf("Name = '%s'\n", name.toUtf8().data());
      printf("Command = '%s'\n", cmd.toUtf8().data());
      printf("Command View = '%s'\n", cmd_view.toUtf8().data());
      printf("Dependences = '%s'\n", dependmask.toUtf8().data());
      printf("Memory: %d bytes\n", calcWeight());
   }
   else
   {
      printf("N'%s' C'%s'", name.toUtf8().data(), cmd.toUtf8().data());
      if( cmd_view.isEmpty() == false) printf(" V'%s'", cmd_view.toUtf8().data());
      if( dependmask.isEmpty() == false) printf(" D'%s'", dependmask.toUtf8().data());
      printf(" %d bytes\n", calcWeight());
   }
}
