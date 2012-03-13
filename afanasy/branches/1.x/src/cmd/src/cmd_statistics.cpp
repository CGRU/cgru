#include "cmd_statistics.h"

#include <communications.h>

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

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

bool CmdStatistics::processArguments( int argc, char** argv, af::Msg &msg)
{
   if( argc >= 1 )
   {
      bool ok; columns = (QString::fromUtf8(argv[0])).toInt( &ok);
      if( false == ok ) return false;
   }
   if( argc >= 2 )
   {
      bool ok; sorting = (QString::fromUtf8(argv[1])).toInt( &ok);
      if( false == ok ) return false;
   }
   msg.set( af::Msg::TStatRequest);
   return true;
}

void CmdStatistics::msgOut( af::Msg& msg)
{
   com::statread( &msg);
   com::statout( columns, sorting);
}
