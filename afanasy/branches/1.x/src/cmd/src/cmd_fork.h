#pragma once

#include "cmd.h"

class CmdFork : public Cmd
{
public:
   CmdFork();
   ~CmdFork();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
