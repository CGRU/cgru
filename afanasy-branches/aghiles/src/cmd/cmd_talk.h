#pragma once

#include "cmd.h"

class CmdTalkList : public Cmd
{
public:
   CmdTalkList();
   ~CmdTalkList();
   bool processArguments( int argc, char** argv, af::Msg &msg);
   void msgOut( af::Msg& msg);
};
