#include "cmd_numeric.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdNumeric::CmdNumeric()
{
   setCmd("num");
   setArgsCount(3);
   setInfo("Test numeric command fill with numbers.");
   setHelp("num [command] [number1] [number2] Fill numeric command with two provided numbers.");
}

CmdNumeric::~CmdNumeric(){}

bool CmdNumeric::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string command = argv[0];
   int number1 = atoi(argv[1]);
   int number2 = atoi(argv[2]);

   std::cout << af::fillNumbers( command, number1, number2);
   std::cout << std::endl;
   return true;
}
