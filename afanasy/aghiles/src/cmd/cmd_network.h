#pragma once

#include "cmd.h"

class CmdNetwork : public Cmd
{
public:
   CmdNetwork();
   ~CmdNetwork();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
