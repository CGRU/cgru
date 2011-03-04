#include "apitask.h"

#include "apiblock.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afapi;

Task::Task()
{
AFINFA("API: Task constuctor called.\n");
   name = AFJOB::TASK_DEFAULT_NAME;
}

Task::~Task()
{
AFINFA("API: Task['%s'] destructor called\n", name.c_str());
}

void Task::stdOut( bool full) const
{
   af::TaskData::stdOut( full);
}

void Task::setName(        const char * str) { name        = str;  }
void Task::setCommand(     const char * str) { command     = str;  }
void Task::setFiles(       const char * str) { files       = str;  }
void Task::setCustomData(  const char * str) { customdata  = str;  }
bool Task::setDependMask(  const char * str)
{
   dependmask =  str;
   return true;
}
