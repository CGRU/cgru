#pragma once

#include "cmd.h"

class CmdVerbose : public Cmd
{
public:
   CmdVerbose();
   ~CmdVerbose();
   bool v_processArguments( int argc, char** argv, af::Msg &msg) final;
};

class CmdHelp : public Cmd
{
public:
   CmdHelp();
   ~CmdHelp();
   bool v_processArguments( int argc, char** argv, af::Msg &msg) final;
};

#ifndef WINNT
class CmdFork : public Cmd
{
public:
   CmdFork();
   ~CmdFork();
   bool v_processArguments( int argc, char** argv, af::Msg &msg) final;
};
#endif
