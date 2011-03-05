#include "cmd_regexp.h"

#include "../libafanasy/regexp.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdRegExp::CmdRegExp()
{
   setCmd("rx");
   setArgsCount(2);
   setInfo("Test a regular expression.");
   setHelp("rx [string] [expression] Whether string matches specified regular expression.");
}

CmdRegExp::~CmdRegExp(){}

bool CmdRegExp::processArguments( int argc, char** argv, af::Msg & msg)
{
   af::RegExp rx;
   std::string str = argv[0];
   std::string pattern = argv[1];
   std::string str_error;
//   rx.setExclude();
//   rx.setContain();
//   rx.setCaseInsensitive();
   if( rx.setPattern( pattern, &str_error)) printf( rx.match( str) ? "   MATCH\n" : "   NOT MATCH\n");
   if( false == str_error.empty()) std::cout << "RegExp Error: " << str_error << std::endl;
   return true;
}
