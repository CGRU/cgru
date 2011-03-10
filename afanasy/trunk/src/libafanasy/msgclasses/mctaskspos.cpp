#include "mctaskspos.h"

#include <stdio.h>

#include "../name_af.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../../include/macrooutput.h"

using namespace af;

MCTasksPos::MCTasksPos( int job_id, const std::string & Message):
   jobid( job_id),
   has_tasks( false ),
   message( Message)
{
}

MCTasksPos::MCTasksPos( Msg * msg)
{
   read( msg);
}

MCTasksPos::~MCTasksPos()
{
}

bool MCTasksPos::addBlock(int numblock)
{
   if( numbloks.size() != 0 )
   {
      if( has_tasks )
      {
         AFERROR("MCTasksPos::addBlock: Message has tasks.\n");
         return false;
      }
   }
   numbloks.push_back( numblock);
   return true;
}

bool MCTasksPos::addTask( int numblock, int numtask)
{
   if( numbloks.size() == 0 ) has_tasks = true;
   else
   {
      if( has_tasks == false )
      {
         AFERROR("MCTasksPos::addTask: Message has blocks only.\n");
         return false;
      }
   }
   numbloks.push_back( numblock);
   numtasks.push_back( numtask);
   return true;
}

void MCTasksPos::readwrite( Msg * msg)
{
   MsgClassUserHost::readwrite( msg);

   rw_int32_t( jobid,      msg);
   rw_bool(    has_tasks,  msg);
   rw_String(  message,    msg);

   rw_Int32_Vect( numbloks, msg);

   if( has_tasks == false) return;

   rw_Int32_Vect( numtasks, msg);
}

void MCTasksPos::generateInfoStream( std::ostringstream & stream, bool full) const
{
   MsgClassUserHost::generateInfoStream( stream, full);
   stream << "\nJob id = " << jobid;
   for( unsigned p = 0; p < numbloks.size(); p++)
   {
      stream << "\n   numblock = " << numbloks[p];
      if( has_tasks) stream << ", numtask = " << numtasks[p];
   }
   stream << "\nMessage: " + message;
}
