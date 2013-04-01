#pragma once

#include "cmd.h"

class CmdStatistics : public Cmd
{
public:
   CmdStatistics();
   ~CmdStatistics();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
   void v_msgOut( af::Msg& msg);
private:
   int columns;
   int sorting;
};
