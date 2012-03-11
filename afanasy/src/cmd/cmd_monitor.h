#pragma once

#include "cmd.h"

class CmdMonitorList : public Cmd
{
public:
   CmdMonitorList();
   ~CmdMonitorList();
   bool processArguments( int argc, char** argv, af::Msg &msg);
   void msgOut( af::Msg& msg);
};
