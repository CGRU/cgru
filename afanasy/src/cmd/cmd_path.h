#pragma once

#include "cmd.h"

class CmdPath : public Cmd
{
public:
   CmdPath();
   ~CmdPath();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
