#pragma once

#include "cmd.h"

class CmdTest : public Cmd
{
public:
   CmdTest();
   ~CmdTest();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
   void v_msgOut( af::Msg& msg);
};
