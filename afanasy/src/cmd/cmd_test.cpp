#include "cmd_test.h"

#include "../libafanasy/msgclasses/mctest.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdTest::CmdTest()
{
   setCmd("test");
   setArgsCount(2);
   setInfo("Test message.");
   setHelp("test [string] [count] Generate some strings. For debug purposes.");
   setMsgType( af::Msg::TTESTDATA);
}

CmdTest::~CmdTest(){}

bool CmdTest::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string qstring( argv[0]);
   int count = atoi(argv[1]);
   af::MCTest mctest( count, qstring);
   for( int i = 0; i < count; i++) mctest.addString( af::itos(i) + ": " + qstring);
   msg.set( af::Msg::TTESTDATA, &mctest);
   if( Verbose ) msg.stdOutData();
   return true;
}

void CmdTest::v_msgOut( af::Msg& msg) {}
