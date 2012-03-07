#pragma once

#include "cmd.h"

class CmdMsg: public Cmd
{
public:
   CmdMsg();
   ~CmdMsg();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
