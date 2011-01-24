#include "apiblock.h"

#include "apijob.h"
#include "apitask.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afapi;

Block::Block()
{
AFINFA("API: Block constuctor called.\n");
   name     = AFJOB::BLOCK_DEFAULT_NAME;
   service  = AFJOB::BLOCK_DEFAULT_SERVICE;
   parser   = AFJOB::BLOCK_DEFAULT_PARSER;
}

Block::~Block()
{
AFINFA("API: Block['%s'] destructor called\n", name.toUtf8().data());
// Do not delete tasks with block. They must be deleted manually. Python will delete them.
   if(tasksdata) for( int b = 0; b < tasksnum; b++) tasksdata[b] = NULL;
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
void Block::setMaxRunningTasksNumber(  int value   )  { maxrunningtasks = value; }
void Block::setTasksMaxRunTime(        int value   )  { tasksmaxruntime = value; }
void Block::setParserCoeff(            int value   )  { parsercoeff     = value; }

void Block::setName(             const char * str) {  name        = QString::fromUtf8(str);  }
void Block::setService(          const char * str) {  service     = QString::fromUtf8(str);  }
void Block::setParser(           const char * str) {  parser      = QString::fromUtf8(str);  }
void Block::setTasksName(        const char * str) {  tasksname   = QString::fromUtf8(str);  }
void Block::setCmdPre(           const char * str) {  cmd_pre     = QString::fromUtf8(str);  }
void Block::setCmdPost(          const char * str) {  cmd_post    = QString::fromUtf8(str);  }
void Block::setCommand(          const char * str) {  command     = QString::fromUtf8(str);  }
void Block::setFiles(            const char * str) {  files       = QString::fromUtf8(str);  }
void Block::setWorkingDirectory( const char * str) {  wdir        = QString::fromUtf8(str);  }
void Block::setEnvironment(      const char * str) {  environment = QString::fromUtf8(str);  }
void Block::setCustomData(       const char * str) {  customdata  = QString::fromUtf8(str);  }

bool Block::setDependMask( const char* str )
{
   QString qstr = QString::fromUtf8( str);
   return af::BlockData::setDependMask( qstr);
}
bool Block::setTasksDependMask( const char* str )
{
   QString qstr = QString::fromUtf8( str);
   return af::BlockData::setTasksDependMask( qstr);
}
bool Block::setHostsMask( const char* str )
{
   QString qstr = QString::fromUtf8( str);
   return af::BlockData::setHostsMask( qstr);
}
bool Block::setHostsMaskExclude( const char* str)
{
   QString qstr = QString::fromUtf8( str);
   return af::BlockData::setHostsMaskExclude( qstr);
}
bool Block::setNeedProperties( const char* str)
{
   QString qstr = QString::fromUtf8( str);
   return af::BlockData::setNeedProperties( qstr);
}

bool Block::setNumeric( int start, int end, int perHost, int increment)
{
   return af::BlockData::setNumeric(  start, end, perHost, increment);
}

void Block::setFramesPerTask( int perTask)
{
   af::BlockData::setFramesPerTask( perTask);
}

void Block::setFileSizeCheck( int min, int max)
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

void Block::setMultiHost( int min, int max, int waitmax, bool sameHostMaster, const char * service, int waitsrv)
{
   if( min < 1)
   {
      AFERROR("Block::setMultiHost: Minimum must be greater then zero.\n");
      return;
   }
   if( max < min)
   {
      AFERROR("Block::setMultiHost: Maximum must be greater or equal then minimum.\n");
      return;
   }
   if((min > AFJOB::TASK_MULTIHOSTMAXHOSTS) || (max > AFJOB::TASK_MULTIHOSTMAXHOSTS))
   {
      AFERRAR("Block::setMultiHost: Maximum hosts number is limited to %d.\n", AFJOB::TASK_MULTIHOSTMAXHOSTS);
      return;
   }
   if( sameHostMaster && service)
      if( QString::fromUtf8(service).isEmpty() == false)
      {
         AFERROR("Block::setMultiHost: Block can't have multihost service if master and slave can be the same host.\n");
         sameHostMaster = false;
      }

   flags = flags | FMultiHost;
   if( sameHostMaster) flags = flags | FSameHostMaster;
   multihost_min  = min;
   multihost_max  = max;
   multihost_waitmax = waitmax;
   multihost_waitsrv = waitsrv;
   if( service) multihost_service = QString::fromUtf8(service);
}

void Block::clearTasksList()
{
   if( isNumeric()) return;
   tasks.clear();
   tasksnum = 0;
}

bool Block::appendTask( Task * task)
{
AFINFO("API: Block::appendTask:\n");
   if( isNumeric())
   {
      AFERROR("Block::appendTask: Can't add a task. Block is numeric!\n");
      return false;
   }

   // Check for unique task object
   for( std::list<Task*>::const_iterator it = tasks.begin(); it != tasks.end(); it++)
      if( task == *it)
      {
         AFERRAR("Block::appendTask: Block already has a task '%s'. Skipping.\n", task->getName().toUtf8().data());
         return false;
      }

   // Get task unique name:
   int suffix_num = 1;
   QString newName( task->getName());
   for(;;)
   {
      bool dubname = false;
      for( std::list<Task*>::const_iterator it = tasks.begin(); it != tasks.end(); it++)
      {
         if( newName == (*it)->getName())
         {
            newName = task->getName() + QString::number( suffix_num++);
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
AFINFO("API: Block::calcWeight:\n");
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
AFINFO("API: Block::readwrite: Deleting old tasks data array.\n");
      delete [] tasksdata;
   }
AFINFO("API: Block::readwrite: Creating tasks data array.\n");
   tasksdata = new af::TaskData*[tasksnum];
   std::list<Task*>::iterator it = tasks.begin();
   for( int t = 0; t < tasksnum; t++, it++) tasksdata[t] = *it;
}
