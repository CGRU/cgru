#include "cmd_farm.h"

#include "../libafanasy/farm.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdFarm::CmdFarm()
{
   setCmd("farm");
   setInfo("Print farm from file.");
   setArgsCount(1);
   setHelp("farm [filename] Print farm information from file.");
}
CmdFarm::~CmdFarm(){}
bool CmdFarm::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string filename( argv[0]);
   af::loadFarm( filename,  true);
   return true;
}

CmdFarmLoad::CmdFarmLoad()
{
   setCmd("fload");
   setInfo("Request server to reload farm file.");
   setHelp("fload Request server to reload farm information.");
   setMsgType( af::Msg::TFarmLoad);
   setRecieving();
}
CmdFarmLoad::~CmdFarmLoad(){}
bool CmdFarmLoad::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   msg.set( af::Msg::TFarmLoad);
   return true;
}

CmdFarmCheck::CmdFarmCheck()
{
   setCmd("fcheck");
   setInfo("Check render name and host type.");
   setHelp("fcheck [render #1] [render #1] ... Check render name(s) if provided, or read name(s) from input.");
}
CmdFarmCheck::~CmdFarmCheck(){}
bool CmdFarmCheck::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   if( af::farm() == NULL) return true;
   if( af::farm()->isValid() == false) return true;

   for( int i = 0; i < argc; i++)
   {
      std::string hostname( argv[i]);
      std::cout << hostname << ": ";
      check( hostname);
   }
   if( argc ) return true;
   static const int maxsize = 1 << 10;
   char buffer[maxsize];
   for(;;)
   {
      int size = read( 0, buffer, maxsize);
      if( size == 0) break;
      if( size < 0)
      {
         AFERRPE("Read input:")
         break;
      }
      std::string hostname( buffer, size-1);
      check( hostname);
   }
   return true;
}
bool CmdFarmCheck::check( const std::string & hostname)
{
   if( hostname.empty() ) return false;
   std::string name, description;
   if( af::farm()->getHost( hostname, host, name, description))
   {
      std::cout << "\"" << name << " - " << description;
      std::cout << std::endl;
      host.v_stdOut( true);
      host.printServices();
      return true;
   }
   else
   {
      std::cout << "Does not exist in farm !" << std::endl;
      return false;
   }
}
