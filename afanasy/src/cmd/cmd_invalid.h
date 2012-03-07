#pragma once

#include "cmd.h"

class CmdInvalid : public Cmd
{
public:
   CmdInvalid();
   ~CmdInvalid();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
