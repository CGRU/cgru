#include "cmd_regexp.h"

#include <QtCore/QRegExp>
#include <QtCore/QString>

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

bool CmdRegExp::processArguments( int argc, char** argv, af::Msg &msg)
{
   QString str = QString::fromUtf8(argv[0]);
   QRegExp rx( argv[1], Qt::CaseInsensitive);
   if( rx.isValid() == false )
   {
      printf("QRegExp: %s\n", rx.errorString().toUtf8().data());
      return false;
   }
   printf( str.contains(rx) ? " MATCH\n" : " NOT MATCH\n");
   return true;
}
