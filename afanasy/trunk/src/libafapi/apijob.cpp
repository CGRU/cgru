#include "apijob.h"

#include <stdio.h>
#include <string.h>

#include "apiblock.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afapi;

Job::Job():
   af::Job( 0),
   message( NULL)
{
   name = AFJOB::DEFAULT_NAME;
}
Job::~Job()
{
   if( message ) delete message;
// Do not delete blocks with job. They must be deleted manually. Python will delete them.
   for( int b = 0; b < blocksnum; b++) blocksdata[b] = NULL;
}

void Job::setUserName(        const char * str     )  { username     = str;   }
void Job::setHostName(        const char * str     )  { hostname     = str;   }
void Job::setName(            const char * str     )  { name         = str;   }
void Job::setCmdPre(          const char * str     )  { cmd_pre      = str;   }
void Job::setCmdPost(         const char * str     )  { cmd_post     = str;   }
void Job::setDescription(     const char * str     )  { description  = str;   }
void Job::setMaxRunningTasks(       int value   )  { maxrunningtasks = value;                               }
void Job::setPriority(              int value   )  { priority        = value;                               }
void Job::setWaitTime(              int value   )  { time_wait       = value;                               }
void Job::offline()                                { state           = state | AFJOB::STATE_OFFLINE_MASK;   }

int  Job::getDataLen() { if( message) return message->writeSize(); else return -1;}
void Job::clearBlocksList() { blocks.clear();}

bool Job::setHostsMask(    const char* str )
{
   return af::Job::setHostsMask( str);
}
bool Job::setHostsMaskExclude(    const char* str )
{
   return af::Job::setHostsMaskExclude( str);
}
bool Job::setDependMask( const char* str )
{
   return af::Job::setDependMask( str);
}
bool Job::setDependMaskGlobal( const char* str )
{
   return af::Job::setDependMaskGlobal( str);
}
bool Job::setNeedOS( const char* str )
{
   return af::Job::setNeedOS( str);
}
bool Job::setNeedProperties( const char* str )
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
   if( blocksdata == NULL ) return;
   delete [] blocksdata;
   blocksdata = NULL;
   blocksnum = 0;
}

void Job::fillBlocksDataPointersFromList()
{
   deleteBlocksDataPointers();
   blocksnum = int( blocks.size());
   if( blocksnum == 0)
   {
      AFERROR("Job::fillBlocksDataPointersFromList: Job has no blocks.")
      return;
   }
   blocksdata = new af::BlockData*[blocksnum];
   std::list<Block*>::iterator it = blocks.begin();
   for( int b = 0; b < blocksnum; b++, it++)
   {
      (*it)->setBlockNumber( b);
      (*it)->fillTasksArrayFromList();
      blocksdata[b] = *it;
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
   for( int b = 0; b < blocksnum; b++) if( blocksdata[b]->isValid() == false) return false;

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
