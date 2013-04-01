#pragma once

#include "cmd.h"

class CmdConfirm : public Cmd
{
public:
   CmdConfirm();
   ~CmdConfirm();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
