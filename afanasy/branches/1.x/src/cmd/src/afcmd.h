#pragma once

#include <name_af.h>
#include <msg.h>

#include <name_afsql.h>

#include "cmd.h"

typedef std::list<Cmd*> CmdList;

/// AfCmd class - Afanasy CLI.
/**
*** AfCmd can query some Afanasy.
*** Message is constucted from command line arguments.
*** Connect to Afanasy.
*** Send him a message.
*** Wait for an answer if needed.
**/
class AfCmd
{
public:
   AfCmd();
   ~AfCmd();

   void addCmd( Cmd * cmd);
   void RegisterCommands();

   bool processCommand( int argc, char** argv, af::Msg &msg);  ///< Generate message parsing command line arguments. Return \c true on sucsess.
   inline bool isRecieving() const { return recieving; }

   bool connect();      ///< Connect to Afanasy. Return true on success.
   bool msgSend( af::Msg& msg);  ///< Send message to Afanasy.Return \c true on sucsess.

   bool msgRecv( af::Msg& msg);  ///< Recieve message from Afanasy. Return \c true on sucsess.
   void msgOut(  af::Msg& msg);  ///< Print message information in stdout.

   void disconnect();   ///< Disconnect.

private:

   int socketfd;                 ///< %Client socket descriptor.
   bool recieving;

   CmdList commands;
   Cmd * command;

};
