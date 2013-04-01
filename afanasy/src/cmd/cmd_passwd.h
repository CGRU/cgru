#pragma once

#include "cmd.h"

class CmdPasswd : public Cmd
{
public:
   CmdPasswd();
   ~CmdPasswd();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
