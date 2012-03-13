#include "talkaf.h"

#include <stdio.h>

#include <environment.h>

#include "monitorcontainer.h"

TalkAf::TalkAf( af::Msg * msg, const af::Address * addr):
   af::Talk( msg, addr)
{
}

TalkAf::~TalkAf()
{
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
