#include "cmd_statistics.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdStatistics::CmdStatistics():
   columns(-1),
   sorting(-1)
{
   setCmd("stat");
   setInfo("Messages statistics.");
   setHelp("rpri [name] [priority] Set render priority.");
   setHelp("stat [columns=-1] [sorting=-1] Number of colums to output (-1:all), sort by column (-1:none).");
   setMsgType( af::Msg::TStatRequest);
   setMsgOutType( af::Msg::TStatData);
   setRecieving();
}

CmdStatistics::~CmdStatistics(){}

bool CmdStatistics::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   if( argc >= 1 ) columns = atoi(argv[0]);
   if( argc >= 2 ) sorting = atoi(argv[1]);
   msg.set( af::Msg::TStatRequest);
   return true;
}

void CmdStatistics::v_msgOut( af::Msg& msg)
{
   af::statread( &msg);
   af::statout( columns, sorting);
}
