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

void TaskData::generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( full)
   {
      stream << "Name = '" << name.toUtf8().data() << "'\n";
      stream << "Command = '" << command.toUtf8().data() << "'\n";
      stream << "Files = '%s'" << files.toUtf8().data() << "'\n";
      stream << "Dependences = '" << dependmask.toUtf8().data() << "'\n";
      stream << "Custom Data = '" << customdata.toUtf8().data() << "'\n";
      stream << "Memory: " << calcWeight() << " bytes\n";
   }
   else
   {
      stream << "N'" << name.toUtf8().data() << "' C'" << command.toUtf8().data() << "'";
      if( false == files.isEmpty()     ) stream << " F'" << files.toUtf8().data() << "'";
      if( false == dependmask.isEmpty()) stream << " D'" << dependmask.toUtf8().data() << "'";
      if( false == customdata.isEmpty()) stream << " cd'" << customdata.toUtf8().data() << "'";
      stream << " " << calcWeight() << " bytes";
      stream << std::endl;
   }
}
