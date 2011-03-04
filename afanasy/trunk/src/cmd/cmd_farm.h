#pragma once

#include "../libafanasy/host.h"

#include "cmd.h"

class CmdFarm : public Cmd
{
public:
   CmdFarm();
   ~CmdFarm();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};

class CmdFarmLoad : public Cmd
{
public:
   CmdFarmLoad();
   ~CmdFarmLoad();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};

class CmdFarmCheck : public Cmd
{
public:
   CmdFarmCheck();
   ~CmdFarmCheck();
   bool processArguments( int argc, char** argv, af::Msg &msg);
   bool check( const std::string & hostname);
   af::Host host;
};
