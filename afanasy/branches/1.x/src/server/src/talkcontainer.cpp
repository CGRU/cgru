#include "talkcontainer.h"

#include <afanasy.h>
#include <msgclasses/mctalkmessage.h>
#include <msgclasses/mctalkdistmessage.h>

#include "msgaf.h"
#include "monitorcontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

TalkContainer::TalkContainer():
   ClientContainer( AFTALK::MAXCOUNT)
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
      af::printTime();
      printf(" : Talk registered: ");
      newTalk->stdOut( false );
      if( monitoring) monitoring->addEvent( af::Msg::TMonitorTalksAdd, id);
      MsgAf* tmp = generateList( af::Msg::TTalksList);
      tmp->setAddress( newTalk);
      tmp->dispatch();
   }
   return new MsgAf( af::Msg::TTalkId, id);
}

void TalkContainer::distributeData( MsgAf *msg)
{
   af::MCTalkdistmessage msgdist( msg);
   const QStringList *list = msgdist.getList();

   QString user, text;
   msgdist.getUser( user);
   msgdist.getText( text);
   int users_quantity = list->size();

   af::MCTalkmessage mcTalkmsg( user, text);
   MsgAf* message = new MsgAf( af::Msg::TTalkData, &mcTalkmsg);
#ifdef _DEBUG
printf("%s> %s\n", user.toUtf8().data(), text.toUtf8().data());
#endif
   TalkContainerIt talksIt( this);
   for( af::Talk *talk = talksIt.Talk(); talk != NULL; talksIt.next(), talk = talksIt.Talk())
   {
      for( int u = 0; u < users_quantity; u++)
      {
#ifdef _DEBUG
printf("%s - %s", talk->getUserName().toUtf8().data(), (*list)[u].toUtf8().data());
#endif
         if( talk->getUserName() == (*list)[u])
         {
#ifdef _DEBUG
printf(" - MATCH !\n", talk->getUserName().toUtf8().data());
#endif
            message->addAddress( talk);
         }
#ifdef _DEBUG
else printf("\n");
#endif
      }
   }
   message->dispatch();
}

//##############################################################################
TalkContainerIt::TalkContainerIt( TalkContainer* container, bool skipZombies):
   AfContainerIt( (AfContainer*)container, skipZombies)
{
}

TalkContainerIt::~TalkContainerIt()
{
}
