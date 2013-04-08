#include "cmd_talk.h"

#include "../libafanasy/msgclasses/mcafnodes.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

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

bool CmdTalkList::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   msg.set( af::Msg::TTalksListRequest);
   return true;
}

void CmdTalkList::v_msgOut( af::Msg& msg)
{
   af::MCAfNodes list( &msg);
   list.v_stdOut( Verbose);
}
