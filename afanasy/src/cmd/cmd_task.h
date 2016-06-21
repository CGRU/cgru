#pragma once

#include "cmd.h"

class CmdTaskLog: public Cmd { public:
   CmdTaskLog();
   ~CmdTaskLog();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdTaskOutput: public Cmd { public:
   CmdTaskOutput();
   ~CmdTaskOutput();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdTaskRestart: public Cmd { public:
   CmdTaskRestart();
   ~CmdTaskRestart();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
