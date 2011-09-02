#pragma once

#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mcafnodes.h"
#include "api.h"

namespace afapi
{
class Cmd
{
public:
   Cmd();
   ~Cmd();
/// Get joblist for all
   bool GetJobList();
/// Get joblist for user id
   bool GetJobListUserId(int userID);
/// Get job info from job Id
   bool GetJobInfo(int jobID);
/// Delete job from jobmask
   bool JobDelete( const std::string & jobMask);
/// Get raw job data, to send to server socket ( call after successfull).

   void setNimby(    const std::string & renderMask); ///< Set "nimby"
   void setNIMBY(    const std::string & renderMask); ///< Set "NIMBY"
   void setFree(     const std::string & renderMask); ///< Set render free
   void ejectTasks(  const std::string & renderMask); ///< Eject running tasks

   char * getData();
/// Get raw job data length.
   int getDataLen();
private:
   af::Msg * message;
};
}
