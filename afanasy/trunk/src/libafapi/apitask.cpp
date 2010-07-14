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
AFINFA("API: Task['%s'] destructor called\n", name.toUtf8().data());
}

void Task::stdOut( bool full) const
{
   af::TaskData::stdOut( full);
}

void Task::setName(        const char * str) { name        = QString::fromUtf8(str);  }
void Task::setCommand(     const char * str) { command     = QString::fromUtf8(str);  }
void Task::setFiles(       const char * str) { files       = QString::fromUtf8(str);  }
void Task::setCustomData(  const char * str) { customdata  = QString::fromUtf8(str);  }
bool Task::setDependMask(  const char * str)
{
   dependmask  = QString::fromUtf8(str);
   return true;
}
