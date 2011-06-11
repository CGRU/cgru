#include "apiblock.h"

#include "apijob.h"
#include "apitask.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afapi;

Block::Block( bool DeleteTasksWithBlock):
      deleteTasksWithBlock( DeleteTasksWithBlock)
{
AFINFA("API: Block constuctor called.")
   name     = AFJOB::BLOCK_DEFAULT_NAME;
   service  = AFJOB::BLOCK_DEFAULT_SERVICE;
   parser   = AFJOB::BLOCK_DEFAULT_PARSER;
}

Block::~Block()
{
AFINFA("API: Block['%s'] destructor called.", name.c_str())
// Do not delete tasks with block. They must be deleted manually. Python will delete them.
   if(( false == deleteTasksWithBlock) && tasksdata ) for( int b = 0; b < tasksnum; b++) tasksdata[b] = NULL;
}

void Block::stdOut( bool full) const
{
   af::BlockData::stdOut( full);
   if( full ) for( std::list<Task*>::const_iterator it = tasks.begin(); it != tasks.end(); it++) (*it)->stdOut( false);
}

bool Block::setCapacity( int value) { return af::BlockData::setCapacity( value); }

void Block::setNeedMemory(             int value   )  { need_memory     = value; }
void Block::setNeedPower(              int value   )  { need_power      = value; }
void Block::setNeedHDD(                int value   )  { need_hdd        = value; }
void Block::setTasksMaxRunTime(        int value   )  { tasksmaxruntime = value; }
void Block::setParserCoeff(            int value   )  { parsercoeff     = value; }

void Block::setName(             const std::string & str) {  name        = str;   }
void Block::setService(          const std::string & str) {  service     = str;   }
void Block::setParser(           const std::string & str) {  parser      = str;   }
void Block::setTasksName(        const std::string & str) {  tasksname   = str;   }
void Block::setCmdPre(           const std::string & str) {  cmd_pre     = str;   }
void Block::setCmdPost(          const std::string & str) {  cmd_post    = str;   }
void Block::setCommand(          const std::string & str) {  command     = str;   }
void Block::setFiles(            const std::string & str) {  files       = str;   }
void Block::setWorkingDirectory( const std::string & str) {  wdir        = str;   }
void Block::setEnvironment(      const std::string & str) {  environment = str;   }
void Block::setCustomData(       const std::string & str) {  customdata  = str;   }

bool Block::setDependMask( const std::string & str )
{
   return af::BlockData::setDependMask( str);
}
bool Block::setTasksDependMask( const std::string & str )
{
   return af::BlockData::setTasksDependMask( str);
}
bool Block::setSubTaskDependMask( const std::string & str )
{
   return af::BlockData::setSubTaskDependMask( str);
}
bool Block::setHostsMask( const std::string & str )
{
   return af::BlockData::setHostsMask( str);
}
bool Block::setHostsMaskExclude( const std::string & str)
{
   return af::BlockData::setHostsMaskExclude( str);
}
bool Block::setNeedProperties( const std::string & str)
{
   return af::BlockData::setNeedProperties( str);
}

bool Block::setNumeric( long long start, long long end, long long perHost, long long increment)
{
   return af::BlockData::setNumeric(  start, end, perHost, increment);
}

void Block::setFramesPerTask( long long perTask)
{
   af::BlockData::setFramesPerTask( perTask);
}

void Block::setFileSizeCheck( long long min, long long max)
{
   filesize_min = min;
   filesize_max = max;
}

void Block::setVariableCapacity( int min, int max)
{
   if( min < 0 ) min = 0;
   if( max < 0 ) max = 0;
   flags = flags | FVarCapacity;
   capcoeff_min = min;
   capcoeff_max = max;
}

void Block::setMultiHost( int min, int max, int waitmax, bool sameHostMaster, const std::string & service, int waitsrv)
{
   if( min < 1)
   {
      AFERROR("Block::setMultiHost: Minimum must be greater then zero.")
      return;
   }
   if( max < min)
   {
      AFERROR("Block::setMultiHost: Maximum must be greater or equal then minimum.")
      return;
   }
   if((min > AFJOB::TASK_MULTIHOSTMAXHOSTS) || (max > AFJOB::TASK_MULTIHOSTMAXHOSTS))
   {
      AFERRAR("Block::setMultiHost: Maximum hosts number is limited to %d.", AFJOB::TASK_MULTIHOSTMAXHOSTS)
      return;
   }
   if( sameHostMaster && ( false == service.empty() ))
   {
      AFERROR("Block::setMultiHost: Block can't have multihost service if master and slave can be the same host.")
      sameHostMaster = false;
   }

   flags = flags | FMultiHost;
   if( sameHostMaster) flags = flags | FSameHostMaster;
   multihost_min  = min;
   multihost_max  = max;
   multihost_waitmax = waitmax;
   multihost_waitsrv = waitsrv;
   if( false == service.empty()) multihost_service = service;
}

void Block::clearTasksList()
{
   if( isNumeric()) return;
   tasks.clear();
   tasksnum = 0;
}

bool Block::appendTask( Task * task)
{
   AFINFO("API: Block::appendTask:")
   if( isNumeric())
   {
      AFERROR("Block::appendTask: Can't add a task. Block is numeric!")
      return false;
   }

   // Check for unique task object
   for( std::list<Task*>::const_iterator it = tasks.begin(); it != tasks.end(); it++)
      if( task == *it)
      {
         AFERRAR("Block::appendTask: Block already has a task '%s'. Skipping.", task->getName().c_str())
         return false;
      }

   // Get task unique name:
   int suffix_num = 1;
   std::string newName( task->getName());
   for(;;)
   {
      bool dubname = false;
      for( std::list<Task*>::const_iterator it = tasks.begin(); it != tasks.end(); it++)
      {
         if( newName == (*it)->getName())
         {
            newName = task->getName() + af::itos( suffix_num++);
            dubname = true;
            break;
         }
         dubname = false;
      }
      if( dubname == false) break;
   }
   task->setName( newName);

   tasks.push_back( task);
   tasksnum++;
   return true;
}

int Block::calcWeight() const
{
   AFINFO("API: Block::calcWeight:")
   int weight = af::BlockData::calcWeight();
   std::list<Task*>::const_iterator tasks_end = tasks.end();
   for( std::list<Task*>::const_iterator it = tasks.begin(); it != tasks_end; it++)
      weight += (*it)->calcWeight();
   return weight;
}

void Block::fillTasksArrayFromList()
{
   if( isNumeric()) return;

   if( tasksdata )
   {
      AFINFO("API: Block::readwrite: Deleting old tasks data array.")
      delete [] tasksdata;
   }
   AFINFO("API: Block::readwrite: Creating tasks data array.")
   tasksdata = new af::TaskData*[tasksnum];
   std::list<Task*>::iterator it = tasks.begin();
   for( int t = 0; t < tasksnum; t++, it++) tasksdata[t] = *it;
}
