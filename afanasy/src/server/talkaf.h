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

   void setZombie();

   bool action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring);

	virtual void v_action( Action & i_action);

   void refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);

private:
};
