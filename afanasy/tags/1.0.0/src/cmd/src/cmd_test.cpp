#include "cmd_test.h"

#include <msgclasses/mctest.h>

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

CmdTest::CmdTest()
{
   setCmd("test");
   setArgsCount(2);
   setInfo("Test message.");
   setHelp("test [string] [count] Generate some strings. For debug purposes.");
   setMsgType( af::Msg::TTESTDATA);
}

CmdTest::~CmdTest(){}

bool CmdTest::processArguments( int argc, char** argv, af::Msg &msg)
{
   QString qstring( argv[0]);
   bool ok;
   int count = (QString::fromUtf8(argv[1])).toInt( &ok);
   if( ok == false ) return false;
   af::MCTest mctest( count, qstring);
   for( int i = 0; i < count; i++) mctest.addString( QString::number(i) + ": " + qstring);
   msg.set( af::Msg::TTESTDATA, &mctest);
   if( Verbose ) msg.stdOutData();
   return true;
}

void CmdTest::msgOut( af::Msg& msg) {}
