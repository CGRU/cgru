#pragma once

#include "cmd.h"

class CmdStatistics : public Cmd
{
public:
   CmdStatistics();
   ~CmdStatistics();
   bool processArguments( int argc, char** argv, af::Msg &msg);
   void msgOut( af::Msg& msg);
private:
   int columns;
   int sorting;
};
