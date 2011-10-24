#pragma once

#include "cmd.h"

class CmdTest : public Cmd
{
public:
   CmdTest();
   ~CmdTest();
   bool processArguments( int argc, char** argv, af::Msg &msg);
   void msgOut( af::Msg& msg);
};
