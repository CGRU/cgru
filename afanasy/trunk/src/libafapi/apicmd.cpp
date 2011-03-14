#include "apicmd.h"

#include <stdio.h>
#include <string.h>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

using namespace afapi;

Cmd::Cmd():
      message( NULL)
{

}

Cmd::~Cmd()
{

}

int  Cmd::getDataLen() { if( message) return message->writeSize(); else return -1;}

char * Cmd::getData()
{
   char * newdata = NULL;
   if( message )
   {
      newdata = new char[message->writeSize()];
      memcpy( newdata, message->buffer(), message->writeSize());
   }
   return newdata;
}

bool Cmd::GetJobListUserId(int userID)
{
   if( message ) delete message;
   message = NULL;

   message = new af::Msg( af::Msg::TJobsListRequestUserId, userID );
   if( message == NULL)
   {
      AFERROR("Cmd::GetJobList: Can't allocate memory for data.\n");
      return false;
   }

   return true;
}

bool Cmd::GetJobList()
{
   if( message ) delete message;
   message = NULL;

   message = new af::Msg( af::Msg::TJobsListRequest );
   if( message == NULL)
   {
      AFERROR("Cmd::GetJobList: Can't allocate memory for data.\n");
      return false;
   }

   return true;
}

bool Cmd::GetJobInfo(int jobID)
{
   if( message ) delete message;
   message = NULL;

   message = new af::Msg( af::Msg::TJobRequestId, jobID );
   if( message == NULL)
   {
      AFERROR("Cmd::TJobRequestId: Can't allocate memory for data.\n");
      return false;
   }

   return true;
}

bool Cmd::JobDelete( const std::string & jobMask)
{
   if( message ) delete message;
   message = NULL;

   message = new af::Msg();

   af::MCGeneral mcgeneral( jobMask, 0);
   message->set( af::Msg::TJobDelete, &mcgeneral);
   if( message == NULL)
   {
      AFERROR("Cmd::TJobDelete: Can't allocate memory for data.\n");
      return false;
   }

   return true;
}
