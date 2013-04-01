#pragma once

#include "cmd.h"

class CmdMsg: public Cmd
{
public:
   CmdMsg();
   ~CmdMsg();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
