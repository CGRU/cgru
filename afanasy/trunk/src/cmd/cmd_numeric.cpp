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
   QString command = QString::fromUtf8(argv[0]);
   int number1, number2; bool ok;
   number1 = (QString::fromUtf8(argv[1])).toInt( &ok);
   if( ok == false ) return false;
   number2 = (QString::fromUtf8(argv[2])).toInt( &ok);
   if( ok == false ) return false;

   printf( "%s\n", af::fillNumbers( command, number1, number2).toUtf8().data());
   return true;
}
