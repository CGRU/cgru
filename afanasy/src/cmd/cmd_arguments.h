#pragma once

#include "cmd.h"

class CmdVerbose : public Cmd
{
public:
   CmdVerbose();
   ~CmdVerbose();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};

class CmdHelp : public Cmd
{
public:
   CmdHelp();
   ~CmdHelp();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};

class CmdProtocol : public Cmd
{
public:
   CmdProtocol();
   ~CmdProtocol();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};

class CmdServerName : public Cmd
{
public:
   CmdServerName();
   ~CmdServerName();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};

class CmdServerPort : public Cmd
{
public:
   CmdServerPort();
   ~CmdServerPort();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};

#ifndef WINNT
class CmdFork : public Cmd
{
public:
   CmdFork();
   ~CmdFork();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
#endif
