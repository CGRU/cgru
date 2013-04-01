#pragma once

#include "cmd.h"

class CmdJSON : public Cmd
{
public:
   CmdJSON();
   ~CmdJSON();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
