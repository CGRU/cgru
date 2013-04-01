#include "cmd_string.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdString::CmdString()
{
   setCmd("str");
   setArgsCount(2);
   setInfo("string operations.");
   setHelp("str [operation] [string] For testing purposes. Operations: split join itos.");
}

CmdString::~CmdString(){}

bool CmdString::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string operation( argv[0]);
   std::string str( argv[1]);
   if( operation == "split")
   {
      std::vector<std::string> strlist = af::strSplit( str);
      std::cout << "Splitted list:";
      for( int i = 0; i < strlist.size(); i++)
         std::cout << "\n   \"" << strlist[i] << "\"";
      std::cout << std::endl;
   }
   else if( operation == "join")
   {
      if( argc < 3 )
      {
         std::cerr << "List arguments to join" << std::endl;
         return false;
      }
      std::list<std::string> strlist;
      strlist.push_back( str);
      for( int i = 2; i < argc; i++) strlist.push_back( argv[i]);
      std::cout << af::strJoin( strlist, "|") << std::endl;
   }
   else if( operation == "itos")
   {
      int number = atoi( str.c_str());
      std::cout << "\"" << af::itos( number) << "\"" << std::endl;
   }
   else
   {
      std::cerr << "Unknown operation \"" << operation << "\"" << std::endl;
      return false;
   }
   return true;
}
