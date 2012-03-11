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

   inline void setUserName( const std::string & value) { username = value;} ///< Set user name for logging
   inline void setHostName( const std::string & value) { hostname = value;} ///< Set host name for logging

/// Get joblist for all
   bool GetJobList();
/// Get joblist for user id
   bool GetJobListUserId(int userID);
/// Get job info from job Id
   bool GetJobInfo(int jobID);
/// Delete job from jobmask
   bool JobDelete( const std::string & jobMask);
/// Get raw job data, to send to server socket ( call after successfull).

    void renderGetList(         const std::string & renderMask); ///< Get render(s) by names mask
    void renderSetNimby(        const std::string & renderMask, const std::string & string = std::string()); ///< Set "nimby"
    void renderSetNIMBY(        const std::string & renderMask, const std::string & string = std::string()); ///< Set "NIMBY"
    void renderSetFree(         const std::string & renderMask, const std::string & string = std::string()); ///< Set render free
    void renderEjectTasks(      const std::string & renderMask, const std::string & string = std::string()); ///< Eject running tasks
    void renderEjectNotMyTasks( const std::string & renderMask, const std::string & string = std::string()); ///< Eject running tasks
    void renderExit(            const std::string & renderMask, const std::string & string = std::string()); ///< Exit render (will ask server to shutdown it)

   void talkExit( const std::string & renderMask, const std::string & string = std::string()); ///< Exit task (will ask server to shutdown it)

   void monitorExit( const std::string & renderMask, const std::string & string = std::string()); ///< Exit monitor(afwatch) (will ask server to shutdown it)

   char * getData();
/// Get raw job data length.
   int getDataLen();
private:
   void action( int type, af::MCGeneral & mcgeneral);

private:
   af::Msg * message;
   std::string username;
   std::string hostname;
};
}
