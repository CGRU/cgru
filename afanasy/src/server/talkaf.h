#pragma once

#include "../libafanasy/talk.h"

#include "afnodesrv.h"

class Action;
class MonitorContainer;

class TalkAf: public af::Talk, public AfNodeSrv
{
public:

	TalkAf( af::Msg * msg);

	~TalkAf();

	void v_setZombie();

	virtual void v_action( Action & i_action);

	void v_refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);

private:
};
