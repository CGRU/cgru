#pragma once

#include "cmd.h"

class CmdTestMsg : public Cmd
{
public:
   CmdTestMsg();
   ~CmdTestMsg();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
   void v_msgOut( af::Msg& msg);
};

class CmdTestThreads : public Cmd
{
public:
   CmdTestThreads();
   ~CmdTestThreads();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
   void v_msgOut( af::Msg& msg);
};

