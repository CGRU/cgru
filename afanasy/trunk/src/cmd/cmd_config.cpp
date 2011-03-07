#include "cmd_config.h"

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdConfig::CmdConfig()
{
   setCmd("config");
   setInfo("Print config from file.");
   setArgsCount(1);
   setHelp("config [filename] Print config loaded from specified file name.");
}
CmdConfig::~CmdConfig(){}
bool CmdConfig::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string filename( argv[0]);
   af::Environment::load( filename, af::Environment::Verbose);
   return true;
}

CmdConfigLoad::CmdConfigLoad()
{
   setCmd("cload");
   setInfo("Request server to reload config file.");
   setHelp("cload Request server to reload configuration.");
   setMsgType( af::Msg::TConfigLoad);
   setRecieving();
}
CmdConfigLoad::~CmdConfigLoad(){}
bool CmdConfigLoad::processArguments( int argc, char** argv, af::Msg &msg)
{
   msg.set( af::Msg::TConfigLoad);
   return true;
}
