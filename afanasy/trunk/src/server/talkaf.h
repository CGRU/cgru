#pragma once

#include "../libafanasy/talk.h"

class MonitorContainer;

class TalkAf: public af::Talk
{
public:

   TalkAf( af::Msg * msg, const af::Address * addr = NULL);

   ~TalkAf();

   void setZombie();

   void refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);

private:
};
