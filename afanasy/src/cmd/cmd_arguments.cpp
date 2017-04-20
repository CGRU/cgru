#include "cmd_arguments.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdVerbose::CmdVerbose()
{
   setCmd("v");
   setInfo("Set verbose mode.");
}
CmdVerbose::~CmdVerbose(){}
bool CmdVerbose::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   Verbose = true;
   return true;
}

CmdHelp::CmdHelp()
{
   setCmd("h");
   setInfo("Set help mode.");
}
CmdHelp::~CmdHelp(){}
bool CmdHelp::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   Help = true;
   return true;
}

#ifndef WINNT
CmdFork::CmdFork()
{
   setCmd("fork");
   setArgsCount(1);
   setInfo("Fork process.");
   setHelp("fork [count] Fork command process. For debug purposes only !");
}
CmdFork::~CmdFork(){}
bool CmdFork::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   int count = atoi(argv[0]);
   for( int i = 0; i < count; i++) fork();
   return true;
}
#endif
