#pragma once

#include "../libafanasy/talk.h"

class MonitorContainer;

class TalkAf: public af::Talk
{
public:

   TalkAf( af::Msg * msg);

   ~TalkAf();

   void setZombie();

   bool action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring);

   void refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);

private:
};
