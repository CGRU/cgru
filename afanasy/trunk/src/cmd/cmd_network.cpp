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
   setHelp("net [operation] For testing purposes. Operations: if.");
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
   else
   {
      std::cerr << "Unknown operation \"" << operation << "\"" << std::endl;
      return false;
   }
   return true;
}
