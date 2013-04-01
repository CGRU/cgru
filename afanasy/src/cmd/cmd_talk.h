#pragma once

#include "cmd.h"

class CmdTalkList : public Cmd
{
public:
   CmdTalkList();
   ~CmdTalkList();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
   void v_msgOut( af::Msg& msg);
};
