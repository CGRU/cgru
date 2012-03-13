#pragma once

#include <talk.h>

class TalkHost: public af::Talk
{
public:
   inline void setId( int new_id) { id = new_id;}
};
