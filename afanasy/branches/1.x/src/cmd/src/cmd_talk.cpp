#include "cmd_talk.h"

#include <msgclasses/mcafnodes.h>

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

extern bool Verbose;

CmdTalkList::CmdTalkList()
{
   setCmd("tlist");
   setInfo("List of online Talks.");
   setMsgType( af::Msg::TTalksListRequest);
   setMsgOutType( af::Msg::TTalksList);
   setRecieving();
}

CmdTalkList::~CmdTalkList(){}

bool CmdTalkList::processArguments( int argc, char** argv, af::Msg &msg)
{
   msg.set( af::Msg::TTalksListRequest);
   return true;
}

void CmdTalkList::msgOut( af::Msg& msg)
{
   af::MCAfNodes list( &msg);
   list.stdOut( Verbose);
}
