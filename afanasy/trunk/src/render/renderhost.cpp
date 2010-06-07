#include "renderhost.h"

#include <QtNetwork/QTcpSocket>

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "../libafqt/name_afqt.h"

#include "res.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

RenderHost* render;

af::Msg* update_handler_ptr( af::Msg * msg)
{
   return render->updateMsg( msg);
}

RenderHost::RenderHost( int32_t State, uint8_t Priority):
   Render( State, Priority)
{
   setOnline();
   render = this;
   host.os = af::Environment::getPlatform();
   GetResources( host, hres, true);
#ifdef WINNT
   Sleep( 100);
#else
   usleep( 100000);
#endif
   GetResources( host, hres, false);
   host.stdOut( true);
   hres.stdOut( true);
}

RenderHost::~RenderHost()
{
}

af::Msg* RenderHost::updateMsg( af::Msg *msg)
{
   if( msg->type() != af::Msg::TRenderUpdate) return msg;

   GetResources( host, hres, false);

   msg->resetWrittenSize();

//   hres.stdOut();
   hres.readwrite( msg);

   return msg;
}
