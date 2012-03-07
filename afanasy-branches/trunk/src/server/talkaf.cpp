#include "talkaf.h"

#include <stdio.h>

#include "../libafanasy/environment.h"

#include "afcommon.h"
#include "monitorcontainer.h"

TalkAf::TalkAf( af::Msg * msg):
   af::Talk( msg)
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
      af::Msg* msg = new af::Msg( af::Msg::TClientExitRequest);
      msg->setAddress( this);
      AFCommon::QueueMsgDispatch( msg);
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
   AFCommon::QueueLog("Deleting talk: " + generateInfoString( false));

   Node::setZombie();
}
