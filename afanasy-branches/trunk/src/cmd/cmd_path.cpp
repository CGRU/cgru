#include "cmd_path.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdPath::CmdPath()
{
   setCmd("path");
   setArgsCount(2);
   setInfo("Path operations.");
   setHelp("path [operation] [path] For testing purposes. Operations: abs fname fpath up home isdir.");
}

CmdPath::~CmdPath(){}

bool CmdPath::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string operation( argv[0]);
   std::string path( argv[1]);
   if( operation == "abs")
   {
      std::cout << af::pathAbsolute( path) << std::endl;
   }
   else if( operation == "fname")
   {
      af::pathFilterFileName( path);
      std::cout << path << std::endl;
   }
   else if( operation == "fpath")
   {
      af::pathFilter( path);
      std::cout << path << std::endl;
   }
   else if( operation == "home")
   {
      std::cout << "Home = \"" << af::pathHome() << "\"" << std::endl;
   }
   else if( operation == "up")
   {
      std::cout << af::pathUp( path) << std::endl;
   }
   else if( operation == "isdir")
   {
      std::cout << af::pathIsFolder( path) << std::endl;
   }
   else
   {
      std::cerr << "Unknown operation \"" << operation << "\"" << std::endl;
      return false;
   }
   return true;
}
