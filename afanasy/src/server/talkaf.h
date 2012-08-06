#pragma once

#include "../libafanasy/talk.h"

#include "afnodesrv.h"

class MonitorContainer;

class TalkAf: public af::Talk, public AfNodeSrv
{
public:

   TalkAf( af::Msg * msg);

   ~TalkAf();

   void setZombie();

   bool action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring);

   virtual void v_action( const JSON & i_action, const std::string & i_author, std::string & io_changes,
						   AfContainer * i_container, MonitorContainer * i_monitoring);

   void refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);

private:
};
