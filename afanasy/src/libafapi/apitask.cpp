#include "apitask.h"

#include "apiblock.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afapi;

Task::Task()
{
AFINFA("API: Task constuctor called.")
   name = AFJOB::TASK_DEFAULT_NAME;
}

Task::~Task()
{
AFINFA("API: Task['%s'] destructor called", name.c_str())
}

void Task::stdOut( bool full) const
{
   af::TaskData::stdOut( full);
}

void Task::setName(        const std::string & str) { name        = str;  }
void Task::setCommand(     const std::string & str) { command     = str;  }
void Task::setFiles(       const std::string & str) { files       = str;  }
void Task::setCustomData(  const std::string & str) { customdata  = str;  }
bool Task::setDependMask(  const std::string & str)
{
   dependmask =  str;
   return true;
}
