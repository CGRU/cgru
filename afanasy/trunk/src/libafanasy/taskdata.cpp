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
   rw_String(  name,       msg);
   if( name_only) return;

   rw_String( command,    msg);
   rw_String( files,      msg);
   rw_String( dependmask, msg);
   rw_String( customdata, msg);
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
      stream << "Name = '" << name << "'\n";
      stream << "Command = '" << command << "'\n";
      stream << "Files = '%s'" << files << "'\n";
      if( dependmask.size()) stream << "Dependences = '" << dependmask << "'\n";
      if( customdata.size()) stream << "Custom Data = '" << customdata << "'\n";
      stream << "Memory: " << calcWeight() << " bytes\n";
   }
   else
   {
      stream << "N'" << name << "' C'" << command << "'";
      if( files.size()     ) stream << " F'" << files << "'";
      if( dependmask.size()) stream << " D'" << dependmask << "'";
      if( customdata.size()) stream << " cd'" << customdata << "'";
      stream << " " << calcWeight() << " bytes";
      stream << std::endl;
   }
}
