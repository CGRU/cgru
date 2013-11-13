#include "talkaf.h"

#include <stdio.h>

#include "../libafanasy/environment.h"

#include "action.h"
#include "afcommon.h"
#include "monitorcontainer.h"

TalkAf::TalkAf( af::Msg * msg):
	af::Talk( msg),
	AfNodeSrv( this)
{
}

TalkAf::~TalkAf()
{
}

void TalkAf::deregister( MonitorContainer * i_monitors)
{
	AFCommon::QueueLog("Talk deregister: " + v_generateInfoString( false));
	setZombie();
	i_monitors->addEvent( af::Msg::TMonitorTalksDel, getId());
}

void TalkAf::v_action( Action & i_action)
{
	const JSON & operation = (*i_action.data)["operation"];
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
			appendLog("Unknown operation \"" + type + "\" by " + i_action.author);
			return;
		}
		appendLog("Operation \"" + type + "\" by " + i_action.author);
	}
}

void TalkAf::v_refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
//printf("TalkAf::refresh: \"%s\"\n", getName().toUtf8().data());
   if( getTimeUpdate() < (currentTime - af::Environment::getTalkZombieTime()))
   {
      if( monitoring) monitoring->addEvent( af::Msg::TMonitorTalksDel, m_id);
		setZombie();
   }
}

