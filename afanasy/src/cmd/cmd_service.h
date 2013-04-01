#pragma once

#include "cmd.h"

class CmdSrvCapacity : public Cmd
{
public:
   CmdSrvCapacity();
   ~CmdSrvCapacity();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};

class CmdSrvHosts : public Cmd
{
public:
   CmdSrvHosts();
   ~CmdSrvHosts();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
