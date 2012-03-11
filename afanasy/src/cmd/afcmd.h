#pragma once

#include "../libafanasy/name_af.h"
#include "../libafanasy/msg.h"

#include "../libafsql/name_afsql.h"

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

    void msgOutput(  af::Msg& msg);  ///< Print message information in stdout.

private:

    bool recieving;

    CmdList commands;
    Cmd * command;

};
