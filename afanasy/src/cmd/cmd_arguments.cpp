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

CmdProtocol::CmdProtocol()
{
   setCmd("IP");
   setArgsCount(1);
   setInfo("Force protocol.");
   setHelp("IP [4|6] Force protocol to use to connect to server. Server name may be solved with several protocols.");
}
CmdProtocol::~CmdProtocol(){}
bool CmdProtocol::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   if( argv[0][0] == '4')
      Protocol = AF_INET;
   else if( argv[0][0] == '6')
      Protocol = AF_INET6;
   else
      printf("Can't force unknown IP%s protocol.\n", argv[0]);
   return true;
}

CmdServerName::CmdServerName()
{
   setCmd("server");
   setArgsCount(1);
   setInfo("Force server name.");
   setHelp("server [name] Force server name to connect (to try to solve).");
}
CmdServerName::~CmdServerName(){}
bool CmdServerName::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   ServerName = argv[0];
   return true;
}

CmdServerPort::CmdServerPort()
{
   setCmd("port");
   setArgsCount(1);
   setInfo("Force server port.");
   setHelp("port [number] Force server port to connect.");
}
CmdServerPort::~CmdServerPort(){}
bool CmdServerPort::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   int port = atoi(argv[0]);
   ServerPort = port;
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