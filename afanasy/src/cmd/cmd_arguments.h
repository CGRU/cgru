#pragma once

#include "cmd.h"

class CmdVerbose : public Cmd
{
public:
   CmdVerbose();
   ~CmdVerbose();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};

class CmdHelp : public Cmd
{
public:
   CmdHelp();
   ~CmdHelp();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};

#ifndef WINNT
class CmdFork : public Cmd
{
public:
   CmdFork();
   ~CmdFork();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
#endif
