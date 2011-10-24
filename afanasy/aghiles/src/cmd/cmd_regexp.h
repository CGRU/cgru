#pragma once

#include "cmd.h"

class CmdRegExp : public Cmd
{
public:
   CmdRegExp();
   ~CmdRegExp();
   bool processArguments( int argc, char** argv, af::Msg & msg);
};
