#include "cmd_parse.h"

#include "../libafanasy/parser.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdParse::CmdParse()
{
	setCmd("parse");
	setArgsCount(1);
	setInfo("parse input with specified parser class.");
	setHelp("parse [parsername] [numframes=1] Check parser.");
}
CmdParse::~CmdParse(){}
bool CmdParse::processArguments( int argc, char** argv, af::Msg &msg)
{
	int numframes = 1;
	if( argc > 0)
	{
		bool ok; int number = af::stoi( argv[1], &ok);
		if( ok ) numframes = number;
	}

	af::Parser parser( argv[0], numframes);
	if( parser.isInitialized() == false)
	{
		AFERRAR("Parser \"%s\" initialization failed.\n", argv[0]);
		return false;
	}

	int maxsize = 1 << 20;
	char * buffer = new char(maxsize);
	for(;;)
	{
		int size = read( 0, buffer, maxsize);
		if( size == 0) break;
		if( size < 0)
		{
			AFERRPE("Read input:");
			break;
		}
//      write( 1, buffer, size);
//      for( int i = 0; i < size; i++) write( 1, buffer+i, 1);
		int percent          = 0;
		int frame            = 0;
		int percentframe     = 0;
		bool error           = false;
		bool warning         = false;
		bool badresult       = false;
		bool finishedsuccess = false;
		std::string output( buffer, size);
		if( parser.parse( output, percent, frame, percentframe, error, warning, badresult, finishedsuccess))
		{
			printf("PERCENT: %d%%", percent);
			printf("; FRAME: %d", frame);
			printf("; PERCENTFRAME: %d%%", percentframe);
			if( error           ) printf("; ERROR");
			if( warning         ) printf("; WARNING");
			if( badresult       ) printf("; BAD RESULT");
			if( finishedsuccess ) printf("; FINISHED SUCCESS");
			printf("\n");
		}
	}
	 delete buffer;
	 return true;
}
