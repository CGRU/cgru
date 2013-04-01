#include "cmd_numeric.h"

#include "../libafanasy/blockdata.h"
#include "../libafanasy/taskexec.h"
#include "../libafanasy/taskprogress.h"

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

bool CmdNumeric::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string command = argv[0];
   int number1 = atoi(argv[1]);
   int number2 = atoi(argv[2]);

   std::cout << af::fillNumbers( command, number1, number2);
   std::cout << std::endl;
   return true;
}

CmdNumericGen::CmdNumericGen()
{
	setCmd("numgen");
	setArgsCount(1);
	setInfo("Generate tasks numbers sequence.");
	setHelp("numgen [quantity] [start from] Generate tasks numbers sequence.");
}

CmdNumericGen::~CmdNumericGen(){}

bool CmdNumericGen::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	int quantity = atoi(argv[0]);
	if( quantity < 1 )
	{
		AFERROR("Quantity should be a number greater that zero.")
		return false;
	}
	int start_from = 0;
	if( argc > 1 )
	{
		start_from = atoi(argv[1]);	
	}

	printf("Tasks quantity = %d, starting from %d:\n", quantity, start_from);

	af::TaskProgress ** tp = new af::TaskProgress * [quantity];
	for( int i = 0; i < quantity; i++)
	{
		tp[i] = new af::TaskProgress;
		tp[i]->state = AFJOB::STATE_READY_MASK;
	}

	int task;
	while(( task = af::getReadyTaskNumber( quantity, tp, af::BlockData::FNonSequential, start_from)) != -1)
	{
		std::cout << " " << task;
	}
	std::cout << std::endl;

	for( int i = 0; i < quantity; i++)
		delete tp[i];
	delete [] tp;

	return true;
}

CmdNumericCmd::CmdNumericCmd()
{
   setCmd("numcmd");
   setArgsCount(5);
   setInfo("Generate numeric commands.");
   setHelp("numcmd [command] [start] [end] [pertask] [increment(by)] Generate numeric commads.");
}

CmdNumericCmd::~CmdNumericCmd(){}

bool CmdNumericCmd::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string command = argv[0];
   long long start = af::stoi(argv[1]);
   long long end   = af::stoi(argv[2]);
   long long fpt   = af::stoi(argv[3]);
   long long inc   = af::stoi(argv[4]);

   af::BlockData block;
   block.setCommand( command);
   block.setNumeric( start, end, fpt, inc);

   int tasksnum = block.getTasksNum();
   for( int t = 0; t < tasksnum; t++)
   {
      af::TaskExec * taskexec = block.genTask( t);
      std::cout << t << ": \"";
      std::cout << taskexec->getCommand();
      std::cout << "\" - " << taskexec->getFramesNum() << " frames";
      std::cout << std::endl;
      delete taskexec;
   }

   std::cout << "Tasks Quantity = " << tasksnum;
   std::cout << std::endl;

   return true;
}

CmdNumericCalcTask::CmdNumericCalcTask()
{
   setCmd("numcalc");
   setArgsCount(5);
   setInfo("Calculate task number using fame number.");
   setHelp("numcalc [start] [end] [pertask] [increment(by)] [frame] Calculate task number by frame.");
}

CmdNumericCalcTask::~CmdNumericCalcTask(){}

bool CmdNumericCalcTask::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   long long start = af::stoi(argv[0]);
   long long end   = af::stoi(argv[1]);
   long long fpt   = af::stoi(argv[2]);
   long long inc   = af::stoi(argv[3]);
   long long frame = af::stoi(argv[4]);

   af::BlockData block;
   block.setNumeric( start, end, fpt, inc);
   bool inValidRange;
   int tasknum = block.calcTaskNumber( frame, inValidRange);

   std::cout << "Tasks Number = " << tasknum;
   if( false == inValidRange ) std::cout << " (out of range)";
   std::cout << std::endl;

   return true;
}
