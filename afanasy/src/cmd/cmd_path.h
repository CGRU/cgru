#pragma once

#include "cmd.h"

class CmdPath : public Cmd
{
public:
   CmdPath();
   ~CmdPath();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
