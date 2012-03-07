#pragma once

#include "cmd.h"

class CmdString : public Cmd
{
public:
   CmdString();
   ~CmdString();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
