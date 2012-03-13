#pragma once

#include "cmd.h"

class CmdHelp : public Cmd
{
public:
   CmdHelp();
   ~CmdHelp();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
