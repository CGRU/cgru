#include "cmd_service.h"

#include "../libafanasy/service.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdSrvCapacity::CmdSrvCapacity()
{
   setCmd("srvcap");
   setArgsCount(2);
   setInfo("apply capacity to command.");
   setHelp("srvcap [servicename] [command] [capacity]");
}
CmdSrvCapacity::~CmdSrvCapacity(){}
bool CmdSrvCapacity::processArguments( int argc, char** argv, af::Msg &msg)
{
   int capacity = atoi(argv[2]);
   std::string wdir;
   std::string command( argv[1]);
   std::string files;
   std::list<std::string> hosts;

   af::Service service( argv[0], wdir, command, files, capacity);
   if( service.isInitialized() == false)
   {
      AFERRAR("Service '%s' initialization failed.\n", argv[0]);
      return false;
   }

   command = service.getCommand();
   std::cout << "Result Command:";
   std::cout << std::endl;
   std::cout << command;
   std::cout << std::endl;
   return true;
}

CmdSrvHosts::CmdSrvHosts()
{
   setCmd("srvhosts");
   setArgsCount(2);
   setInfo("apply hosts to command.");
   setHelp("srvcap [servicename] [command] [hosts1 host2 ...]");
}
CmdSrvHosts::~CmdSrvHosts(){}
bool CmdSrvHosts::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string command( argv[1]);
   std::list<std::string> hosts;
   for( int a = 2; a < argc; a++) hosts.push_back( argv[a]);

   std::string wdir;
   int capacity = -1;
   std::string files;

   af::Service service( argv[0], wdir, command, files, capacity, hosts);
   if( service.isInitialized() == false)
   {
      AFERRAR("Service '%s' initialization failed.\n", argv[0]);
      return false;
   }

   command = service.getCommand();
   std::cout << "Result Command:";
   std::cout << std::endl;
   std::cout << command;
   std::cout << std::endl;
   return true;
}
