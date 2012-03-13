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
bool CmdFarm::processArguments( int argc, char** argv, af::Msg &msg)
{
   QString filename( argv[0]);
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
bool CmdFarmLoad::processArguments( int argc, char** argv, af::Msg &msg)
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
bool CmdFarmCheck::processArguments( int argc, char** argv, af::Msg &msg)
{
   if( af::farm() == NULL) return true;
   if( af::farm()->isValid() == false) return true;

   for( int i = 0; i < argc; i++)
   {
      QString hostname( argv[i]);
      printf("%s: ", hostname.toUtf8().data());
      check( hostname);
   }
   if( argc ) return true;
   int maxsize = 1 << 10;
   char buffer[maxsize];
   for(;;)
   {
      int size = read( 0, buffer, maxsize);
      if( size == 0) break;
      if( size < 0)
      {
         AFERRPE("Read input:");
         break;
      }
      QString hostname = QString::fromUtf8( buffer, size-1);
      check( hostname);
   }
   return true;
}
bool CmdFarmCheck::check( const QString & hostname)
{
   if( hostname.isEmpty() ) return false;
   QString name, description;
   if( af::farm()->getHost( hostname, host, name, description))
   {
      printf("\"%s\" - %s\n", name.toUtf8().data(), description.toUtf8().data());
      host.stdOut( true);
      return true;
   }
   else
   {
      printf("Does not exists in farm !\n");
      return false;
   }
}
