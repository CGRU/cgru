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
   m_name     = AFJOB::BLOCK_DEFAULT_NAME;
   m_service  = AFJOB::BLOCK_DEFAULT_SERVICE;
   m_parser   = AFJOB::BLOCK_DEFAULT_PARSER;
}

Block::~Block()
{
AFINFA("API: Block['%s'] destructor called.", m_name.c_str())
// Do not delete tasks with block. They must be deleted manually. Python will delete them.
   if(( false == deleteTasksWithBlock) && m_tasks_data ) for( int b = 0; b < m_tasks_num; b++) m_tasks_data[b] = NULL;
}

void Block::stdOut( bool full) const
{
   af::BlockData::stdOut( full);
   if( full ) for( std::list<Task*>::const_iterator it = tasks.begin(); it != tasks.end(); it++) (*it)->stdOut( false);
}

void Block::setVariableCapacity( int min, int max)
{
   if( min < 0 ) min = 0;
   if( max < 0 ) max = 0;
   m_flags = m_flags | FVarCapacity;
   m_capacity_coeff_min = min;
   m_capacity_coeff_max = max;
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

   m_flags = m_flags | FMultiHost;
   if( sameHostMaster) m_flags = m_flags | FSameHostMaster;
   m_multihost_min  = min;
   m_multihost_max  = max;
   m_multihost_waitmax = waitmax;
   m_multihost_waitsrv = waitsrv;
   if( false == service.empty()) m_multihost_service = service;
}

void Block::clearTasksList()
{
   if( isNumeric()) return;
   tasks.clear();
   m_tasks_num = 0;
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
   m_tasks_num++;
   m_frame_last = m_tasks_num;
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

   if( m_tasks_data )
   {
      AFINFO("API: Block::readwrite: Deleting old tasks data array.")
      delete [] m_tasks_data;
   }
   AFINFO("API: Block::readwrite: Creating tasks data array.")
   m_tasks_data = new af::TaskData*[m_tasks_num];
   std::list<Task*>::iterator it = tasks.begin();
   for( int t = 0; t < m_tasks_num; t++, it++) m_tasks_data[t] = *it;
}
