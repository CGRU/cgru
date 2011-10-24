#pragma once

#include "cmd.h"

class CmdTaskOutput: public Cmd { public:
   CmdTaskOutput();
   ~CmdTaskOutput();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
