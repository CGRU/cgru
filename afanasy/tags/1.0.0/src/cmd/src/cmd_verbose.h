#pragma once

#include "cmd.h"

class CmdVerbose : public Cmd
{
public:
   CmdVerbose();
   ~CmdVerbose();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
