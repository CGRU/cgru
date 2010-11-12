#include "talkaf.h"

#include <stdio.h>

#include "../libafanasy/environment.h"

#include "monitorcontainer.h"

TalkAf::TalkAf( af::Msg * msg, const af::Address * addr):
   af::Talk( msg, addr)
{
}

TalkAf::~TalkAf()
{
}

bool TalkAf::action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring)
{
   switch( type)
   {
   case af::Msg::TTalkExit:
   {
      MsgAf* msg = new MsgAf( af::Msg::TClientExitRequest);
      msg->setAddress( this);
      msg->dispatch();
      return true;
   }
   default:
   {
      return false;
   }
   }
   return true;
}

void TalkAf::refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
//printf("TalkAf::refresh: \"%s\"\n", getName().toUtf8().data());
   if( getTimeUpdate() < (currentTime - af::Environment::getTalkZombieTime()))
   {
      if( monitoring) monitoring->addEvent( af::Msg::TMonitorTalksDel, id);
      setZombie();
   }
}

void TalkAf::setZombie()
{
   af::printTime(); printf(" : Deleting Talk: "); stdOut( false);

   Node::setZombie();
}
