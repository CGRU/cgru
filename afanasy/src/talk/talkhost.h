#pragma once

#include "../libafanasy/talk.h"

class TalkHost: public af::Talk
{
public:
   inline void setId( int new_id) { m_id = new_id;}
};
