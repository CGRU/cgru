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

void TalkAf::v_action( const JSON & i_action, const std::string & i_author, std::string & io_changes,
						AfContainer * i_container, MonitorContainer * i_monitoring)
{
	const JSON & operation = i_action["operation"];
	if( operation.IsObject())
	{
		std::string type;
		af::jr_string("type", type, operation);
		if( type == "exit")
		{
			af::Msg* msg = new af::Msg( af::Msg::TClientExitRequest);
			msg->setAddress( this);
			AFCommon::QueueMsgDispatch( msg);
			return;
		}
		else
		{
			appendLog("Unknown operation \"" + type + "\" by " + i_author);
			return;
		}
		appendLog("Operation \"" + type + "\" by " + i_author);
	}
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
      if( monitoring) monitoring->addEvent( af::Msg::TMonitorTalksDel, m_id);
      setZombie();
   }
}

void TalkAf::setZombie()
{
   AFCommon::QueueLog("Deleting talk: " + generateInfoString( false));

   Node::setZombie();
}
