#pragma once

#include "../libafanasy/msg.h"
#include "../libafanasy/name_af.h"

#include "../libafsql/name_afsql.h"

#include "cmd.h"

typedef std::list<Cmd *> CmdList;

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

	void addCmd(Cmd *cmd);
	void RegisterCommands();

	bool processCommand(
		int argc, char **argv,
		af::Msg &msg); ///< Generate message parsing command line arguments. Return \c true on sucsess.
	void msgOutput(af::Msg &msg); ///< Print message information in stdout.

  private:
	CmdList commands;
	Cmd *command;
};
