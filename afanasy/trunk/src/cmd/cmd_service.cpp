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
   bool ok;
   int capacity = (QString::fromUtf8(argv[2])).toInt( &ok);
   if( false == ok )
   {
      AFERRAR("Invalid capacity number '%s'.\n", argv[2]);
      return false;
   }
   QString command( argv[1]);
   QString files;

   af::Service service( argv[0], command, files, true);
   if( service.isInitialized() == false)
   {
      AFERRAR("Service '%s' initialization failed.\n", argv[0]);
      return false;
   }

   command = service.applyCmdCapacity( capacity);
   printf("Result Command:\n%s\n", command.toUtf8().data());
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
   QString command( argv[1]);
   QStringList hosts;
   for( int a = 2; a < argc; a++) hosts << argv[a];

   QString files;

   af::Service service( argv[0], command, files, true);
   if( service.isInitialized() == false)
   {
      AFERRAR("Service '%s' initialization failed.\n", argv[0]);
      return false;
   }

   command = service.applyCmdHosts( hosts);
   printf("Result Command:\n%s\n", command.toUtf8().data());
   return true;
}
