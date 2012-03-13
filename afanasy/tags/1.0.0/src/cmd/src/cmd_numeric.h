#pragma once

#include "cmd.h"

class CmdNumeric : public Cmd
{
public:
   CmdNumeric();
   ~CmdNumeric();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
