#pragma once

#include "../libafanasy/name_af.h"
#include "../libafanasy/render.h"

class QTcpSocket;

class Parser;

af::Msg* update_handler_ptr( af::Msg * msg);

class RenderHost: public af::Render
{
public:
   RenderHost( int32_t State, uint8_t Priority);
   RenderHost();
   ~RenderHost();

   inline void setId( int new_id) { id = new_id;}

   af::Msg* updateMsg( af::Msg *msg);

private:
#ifdef WINNT
   QStringList windowsmustdie;
#endif
};
