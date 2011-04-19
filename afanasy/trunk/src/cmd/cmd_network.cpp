#include "cmd_network.h"

#include "../libafanasy/netif.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdNetwork::CmdNetwork()
{
   setCmd("net");
   setArgsCount(1);
   setInfo("Network operations.");
   setHelp("net [operation] For testing purposes. Operations: [if,ip].");
}

CmdNetwork::~CmdNetwork(){}

bool CmdNetwork::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string operation( argv[0]);
   if( operation == "if")
   {
      std::vector<af::NetIF*> netIFs;
      af::NetIF::getNetIFs( netIFs, true);
      std::cout << "Network Interfaces:" << std::endl;
      for( int i = 0; i < netIFs.size(); i++)
      {
         std::cout << "   ";
         netIFs[i]->stdOut(true);
      }
   }
   if( operation == "ip")
   {
      if( argc < 1 ) return false;
      std::string addr( argv[1]);
      if( af::netIsIpAddr( addr)) printf("IP address string.\n");
      else printf("Not an IP address string.\n");
   }
   else
   {
      std::cerr << "Unknown operation \"" << operation << "\"" << std::endl;
      return false;
   }
   return true;
}
