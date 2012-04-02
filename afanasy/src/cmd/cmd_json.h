#pragma once

#include "cmd.h"

class CmdJSON : public Cmd
{
public:
   CmdJSON();
   ~CmdJSON();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
