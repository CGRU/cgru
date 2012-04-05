#include "apitask.h"

#include "apiblock.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afapi;

Task::Task()
{
AFINFA("API: Task constuctor called.")
   m_name = AFJOB::TASK_DEFAULT_NAME;
}

Task::~Task()
{
AFINFA("API: Task['%s'] destructor called", m_name.c_str())
}

void Task::stdOut( bool full) const
{
   af::TaskData::stdOut( full);
}

void Task::setName(        const std::string & str) { m_name        = str;  }
void Task::setCommand(     const std::string & str) { m_command     = str;  }
void Task::setFiles(       const std::string & str) { m_files       = str;  }
void Task::setCustomData(  const std::string & str) { m_custom_data  = str;  }
bool Task::setDependMask(  const std::string & str)
{
   m_depend_mask =  str;
   return true;
}
