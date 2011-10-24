#pragma once

#include "cmd.h"

class CmdConfig : public Cmd
{
public:
   CmdConfig();
   ~CmdConfig();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};

class CmdConfigLoad : public Cmd
{
public:
   CmdConfigLoad();
   ~CmdConfigLoad();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
