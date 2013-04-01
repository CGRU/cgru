#pragma once

#include "cmd.h"

class CmdParse : public Cmd
{
public:
   CmdParse();
   ~CmdParse();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
