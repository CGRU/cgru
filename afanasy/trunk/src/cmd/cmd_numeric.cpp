#include "cmd_numeric.h"

#include "../libafanasy/blockdata.h"
#include "../libafanasy/taskexec.h"

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

CmdNumericCmd::CmdNumericCmd()
{
   setCmd("numcmd");
   setArgsCount(5);
   setInfo("Test numeric command fill with numbers.");
   setHelp("numcmds [command] [start] [end] [pertask] [increment(by)] Check numeric commad.");
}

CmdNumericCmd::~CmdNumericCmd(){}

bool CmdNumericCmd::processArguments( int argc, char** argv, af::Msg &msg)
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

bool CmdNumericCalcTask::processArguments( int argc, char** argv, af::Msg &msg)
{
   long long start = af::stoi(argv[0]);
   long long end   = af::stoi(argv[1]);
   long long fpt   = af::stoi(argv[2]);
   long long inc   = af::stoi(argv[3]);
   long long frame = af::stoi(argv[4]);

   af::BlockData block;
   block.setNumeric( start, end, fpt, inc);
   int tasknum;
   bool in_range = block.calcTaskNumber( frame, tasknum);

   std::cout << "Tasks Number = " << tasknum;
   if( false == in_range ) std::cout << " (out of range)";
   std::cout << std::endl;

   return true;
}
