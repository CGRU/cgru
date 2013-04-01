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
   setHelp("rx [string] [expression] [eci] Whether string matches specified regular expression\n"
           "Optional mode [eci]: exclude, contain, case insensitive.\n"
           "Examples:\n"
           "afcmd rx ubuntu \"u.*\"\n"
           "afcmd rx ubuntu \"UBU\" eci");
}

CmdRegExp::~CmdRegExp(){}

bool CmdRegExp::v_processArguments( int argc, char** argv, af::Msg & msg)
{
   af::RegExp rx;
   std::string str = argv[0];
   std::string pattern = argv[1];
   std::string str_error;
   if( false == af::RegExp::Validate( pattern, &str_error))
   {
      std::cout << "RegExp Error: \"" << pattern << "\"" << std::endl << str_error << std::endl;
   }
   else
   {
      std::cout << "String:     \"" << str     << "\"" << std::endl;
      std::cout << "Expression: \"" << pattern << "\"" << std::endl;
      if( argc >= 3 )
      {
         char * mode = argv[2];
         int pos = -1;
         while( mode[++pos] != '\0')
         {
            if( mode[pos] == 'e') rx.setExclude();
            if( mode[pos] == 'c') rx.setContain();
            if( mode[pos] == 'i') rx.setCaseInsensitive();
         }
      }
      if( rx.setPattern( pattern, &str_error)) printf( rx.match( str) ? "   MATCH\n" : "   NOT MATCH\n");
      if( false == str_error.empty()) std::cout << "RegExp Error: " << str_error << std::endl;
   }
   return true;
}
