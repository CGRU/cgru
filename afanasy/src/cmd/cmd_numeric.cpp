#include "cmd_numeric.h"

#include "../libafanasy/blockdata.h"
#include "../libafanasy/service.h"
#include "../libafanasy/taskexec.h"
#include "../libafanasy/taskprogress.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdNumericCmd::CmdNumericCmd()
{
	setCmd("numcmd");
	setArgsCount(4);
	setInfo("Test numeric command fill with numbers.");
	setHelp("numcmd [service] [frame_start] [frame_end] [command] Fill service numeric command with start and end frames.");
}
CmdNumericCmd::~CmdNumericCmd(){}
bool CmdNumericCmd::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string type        =      argv[0];
	int         frame_start = atoi(argv[1]);
	int         frame_end   = atoi(argv[2]);
	std::string command     =      argv[3];

	af::Service service(type, command, frame_start, frame_end);
	printf("\"%s\"\n", service.getCommand().c_str());
	return true;
}


CmdNumeric::CmdNumeric()
{
	setCmd("numeric");
	setArgsCount(6);
	setInfo("Generate numeric tasks, find task by frame.");
	setHelp("numeric [start] [end] [pertask] [increment(by)] [sequential] [frame to find] Generate numeric tasks, find task by frame.");
}
CmdNumeric::~CmdNumeric(){}
bool CmdNumeric::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	// Get arguments:
	long long start = af::stoi(argv[0]);
	long long end   = af::stoi(argv[1]);
	long long inc   = af::stoi(argv[2]);
	long long fpt   = af::stoi(argv[3]);
	long long seq   = af::stoi(argv[4]);

	bool find_frame_on = argc > 6;
	long long find_frame = 0;
	if( find_frame_on )
		find_frame = af::stoi(argv[6]);

	// Print arguments:
	std::cout << "first:" << start << " last:" << end << " fpt: " << fpt << " finc:" << inc << " seq:" << seq;
	if( find_frame_on )
		std::cout << " find:" << find_frame;
	std::cout << std::endl;

	// Create block:
	af::BlockData block;
	block.setNumeric( start, end, fpt, inc);
	block.setSequential( seq);

	// Create progresses:
	af::TaskProgress ** tp = new af::TaskProgress * [block.getTasksNum()];
	for( int i = 0; i < block.getTasksNum(); i++)
	{
		tp[i] = new af::TaskProgress;
		tp[i]->state = AFJOB::STATE_READY_MASK;
	}
	// Fill tasks order:
	int * tasks_order = new int[block.getTasksNum()];
	int task; int order = 0;
	while(( task = block.getReadyTaskNumber( tp, 0, NULL)) != -1)
	{
		if( task < 0 )
		{
			AFERRAR("Task number is less than zero = %d (order=%d)", task, order)
			continue;
		}
		if( task >= block.getTasksNum())
		{
			AFERRAR("Task number is less than zero = %d (order=%d)", task, order)
			continue;
		}
		tasks_order[task] = order;
		order++;
	}

	// Find frame:
	int founded_task = 0;
	bool valid_range = false;
	if( find_frame_on )
		founded_task = block.calcTaskNumber( find_frame, valid_range);

	// Ouptut results:
	for( int t = 0; t < block.getTasksNum(); t++)
	{
		af::TaskExec * taskexec = block.genTask( t);
		std::cout << t << ":\t";
		std::cout << taskexec->getFrameStart() << "-" << taskexec->getFrameFinish();
		std::cout << " - " << taskexec->getFramesNum() << " frames";
		std::cout << ", order=" << tasks_order[t];
		if( find_frame_on && ( founded_task == t ))
		{
			std::cout << " - " << find_frame;
			if( ! valid_range )
				std::cout <<  " out-of-range";
		}
		std::cout << std::endl;
		delete taskexec;
	}

	std::cout << "Tasks Quantity = " << block.getTasksNum();
	std::cout << std::endl;

	delete [] tasks_order;

	return true;
}

