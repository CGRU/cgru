#include "cmd_test.h"

#include "../libafanasy/common/dlThread.h"

#include "../libafanasy/msgclasses/mctest.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdTestMsg::CmdTestMsg()
{
	setCmd("tmsg");
	setArgsCount(2);
	setInfo("Test message.");
	setHelp("tmsg [string] [count]\nGenerate some strings. For debug purposes.");
	setMsgType( af::Msg::TTESTDATA);
}

CmdTestMsg::~CmdTestMsg(){}

bool CmdTestMsg::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string qstring( argv[0]);
	int count = atoi(argv[1]);
	af::MCTest mctest( count, qstring);
	for( int i = 0; i < count; i++) mctest.addString( af::itos(i) + ": " + qstring);
	msg.set( af::Msg::TTESTDATA, &mctest);
	if( Verbose ) msg.stdOutData();
	return true;
}

void CmdTestMsg::v_msgOut( af::Msg& msg) {}


CmdTestThreads::CmdTestThreads()
{
	setCmd("tthr");
	setArgsCount(1);
	setInfo("Test threads.");
	setHelp("tthr [count]\nRaise [count] number of threads. For debug purposes.");
}

CmdTestThreads::~CmdTestThreads(){}

void testThread( void * i_args)
{
	int * ints = (int*)(i_args);
	printf("#%09d started.\n", ints[0]);
	af::sleep_sec(1);
	printf("#%09d finished.\n", ints[0]);
	delete [] ints;
}

bool CmdTestThreads::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	int count = atoi(argv[0]);

	for( int i = 0; i < count; i++)
	{
		DlThread *t = new DlThread();
		int * args = new int[1024];
		args[0] = i;

		t->SetDetached();
		t->Start( testThread, args);
	}

	af::sleep_sec(2);

	return true;
}

void CmdTestThreads::v_msgOut( af::Msg& msg) {}

