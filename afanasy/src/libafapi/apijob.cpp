#include "apijob.h"

#include <stdio.h>
#include <string.h>

#include "apiblock.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afapi;

Job::Job( bool DeleteBlocksWithJob):
   af::Job( 0),
   message( NULL),
   deleteBlocksWithJob( DeleteBlocksWithJob)
{
   m_name = AFJOB::DEFAULT_NAME;
}
Job::~Job()
{
   if( message ) delete message;

   // Deleting blocks some times not needed, for example Python deletes all objects itself
   if( false == deleteBlocksWithJob)
      for( int b = 0; b < m_blocksnum; b++) m_blocksdata[b] = NULL;
}

void Job::setUserName(     const std::string & str )  { m_user_name     = str;   }
void Job::setHostName(     const std::string & str )  { m_host_name     = str;   }
void Job::setName(         const std::string & str )  { m_name         = str;   }
void Job::setCmdPre(       const std::string & str )  { m_cmd_pre      = str;   }
void Job::setCmdPost(      const std::string & str )  { m_cmd_post     = str;   }
void Job::setDescription(  const std::string & str )  { m_description  = str;   }
void Job::setMaxRunningTasks(       int value      )  { m_max_running_tasks    = value; }
void Job::setMaxRunTasksPerHost(    int value      )  { m_max_running_tasks_per_host = value; }
void Job::setPriority(              int value      )  { m_priority           = value; }
void Job::setWaitTime(        long long value      )  { m_time_wait          = value; }
void Job::offline()                                   { m_state              = m_state | AFJOB::STATE_OFFLINE_MASK;   }

int  Job::getDataLen() { if( message) return message->writeSize(); else return -1;}
void Job::clearBlocksList() { blocks.clear();}

bool Job::setHostsMask( const std::string & str )
{
   return af::Job::setHostsMask( str);
}
bool Job::setHostsMaskExclude( const std::string & str )
{
   return af::Job::setHostsMaskExclude( str);
}
bool Job::setDependMask( const std::string & str )
{
   return af::Job::setDependMask( str);
}
bool Job::setDependMaskGlobal( const std::string & str )
{
   return af::Job::setDependMaskGlobal( str);
}
bool Job::setNeedOS( const std::string & str )
{
   return af::Job::setNeedOS( str);
}
bool Job::setNeedProperties( const std::string & str )
{
   return af::Job::setNeedProperties( str);
}

void Job::setUniqueBlockName( Block * block)
{
   int block_num = 1;
   std::string newBlockName( block->getName());
   for(;;)
   {
      bool dubname = false;
      for( std::list<Block*>::iterator it = blocks.begin(); it != blocks.end(); it++)
      {
         if( newBlockName == (*it)->getName())
         {
            newBlockName = block->getName() + af::itos( block_num++);
            dubname = true;
            break;
         }
         dubname = false;
      }
      if( dubname == false) break;
   }
   block->setName( newBlockName);
}

bool Job::appendBlock( Block * block)
{
   // Check for unique block object
   for( std::list<Block*>::const_iterator it = blocks.begin(); it != blocks.end(); it++)
      if( block == *it)
      {
         AFERRAR("Job::appendBlock: Job already has a block '%s'. Skipping.", block->getName().c_str())
         return false;
      }

   setUniqueBlockName( block);
   blocks.push_back( block);
   return true;
}

char * Job::getData()
{
   char * newdata = NULL;
   if( message )
   {
      newdata = new char[message->writeSize()];
      memcpy( newdata, message->buffer(), message->writeSize());
   }
   return newdata;
}

void Job::deleteBlocksDataPointers()
{
   if( m_blocksdata == NULL ) return;
   delete [] m_blocksdata;
   m_blocksdata = NULL;
   m_blocksnum = 0;
}

void Job::fillBlocksDataPointersFromList()
{
   deleteBlocksDataPointers();
   m_blocksnum = int( blocks.size());
   if( m_blocksnum == 0)
   {
      AFERROR("Job::fillBlocksDataPointersFromList: Job has no blocks.")
      return;
   }
   m_blocksdata = new af::BlockData*[m_blocksnum];
   std::list<Block*>::iterator it = blocks.begin();
   for( int b = 0; b < m_blocksnum; b++, it++)
   {
      (*it)->setBlockNumber( b);
      (*it)->fillTasksArrayFromList();
      m_blocksdata[b] = *it;
   }
}

bool Job::construct()
{
   if( message ) delete message;
   message = NULL;

   if( blocks.size() == 0)
   {
      AFERROR("Job::construct: Job has no blocks.")
      return false;
   }
   fillBlocksDataPointersFromList();
   for( int b = 0; b < m_blocksnum; b++) if( m_blocksdata[b]->isValid() == false) return false;

   message = new af::Msg( af::Msg::TJobRegister, this);
   if( message == NULL)
   {
      AFERROR("Job::construct: Can't allocate memory for data.")
      return false;
   }
   return true;
}

void Job::output( bool full)
{
   fillBlocksDataPointersFromList();
   stdOut( full);
}
