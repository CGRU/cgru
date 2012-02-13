#include "talkcontainer.h"

#include "../include/afanasy.h"

#include "../libafanasy/msgclasses/mctalkmessage.h"
#include "../libafanasy/msgclasses/mctalkdistmessage.h"
#include "../libafanasy/msgqueue.h"

#include "afcommon.h"
#include "monitorcontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

TalkContainer::TalkContainer():
   ClientContainer( "Talks", AFTALK::MAXCOUNT)
{
}

TalkContainer::~TalkContainer()
{
AFINFO("TalkContainer::~TalkContainer:\n");
}

MsgAf * TalkContainer::addTalk( TalkAf *newTalk, MonitorContainer * monitoring)
{
   int id = addClient( newTalk, true, monitoring, af::Msg::TMonitorTalksDel);
   if( id != 0 )
   {
      AFCommon::QueueLog("Talk registered: " + newTalk->generateInfoString( false));
      if( monitoring) monitoring->addEvent( af::Msg::TMonitorTalksAdd, id);
      MsgAf* msg = generateList( af::Msg::TTalksList);
      msg->setAddress( newTalk);
      AFCommon::QueueMsgDispatch( msg);
   }
   return new MsgAf( af::Msg::TTalkId, id);
}

void TalkContainer::distributeData( MsgAf *msg)
{
   af::MCTalkdistmessage msgdist( msg);
   const std::list<std::string> * list = msgdist.getList();

   std::string user, text;
   msgdist.getUser( user);
   msgdist.getText( text);
//   int users_quantity = list->size();

   af::MCTalkmessage mcTalkmsg( user, text);
   MsgAf* message = new MsgAf( af::Msg::TTalkData, &mcTalkmsg);
#ifdef _DEBUG
printf("%s> %s\n", user.c_str(), text.c_str());
#endif
   TalkContainerIt talksIt( this);
   for( af::Talk *talk = talksIt.Talk(); talk != NULL; talksIt.next(), talk = talksIt.Talk())
   {
      for( std::list<std::string>::const_iterator it = list->begin(); it != list->end(); it++)
      {
#ifdef _DEBUG
printf("%s - %s", talk->getUserName().c_str(), (*it).c_str());
#endif
         if( talk->getUserName() == *it)
         {
#ifdef _DEBUG
printf(" - MATCH !\n", talk->getUserName().c_str());
#endif
            message->addAddress( talk);
         }
#ifdef _DEBUG
else printf("\n");
#endif
      }
   }
   AFCommon::QueueMsgDispatch( msg);
}

//##############################################################################
TalkContainerIt::TalkContainerIt( TalkContainer* container, bool skipZombies):
   AfContainerIt( (AfContainer*)container, skipZombies)
{
}

TalkContainerIt::~TalkContainerIt()
{
}
