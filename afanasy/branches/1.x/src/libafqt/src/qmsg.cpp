#include "qmsg.h"

#include <address.h>

//#define AFOUTPUT
//#undef AFOUTPUT
#include <macrooutput.h>

using namespace afqt;

QMsg::QMsg( int msgType, int msgInt, bool toRecieve, const af::Address * sendTo):
   Msg( msgType, msgInt),
   next_ptr( NULL),
   address( NULL),
   recieve( toRecieve)
{
   if( sendTo ) address = new af::Address( sendTo);
}

QMsg::QMsg( int msgType, Af * afClass, bool toRecieve, const af::Address * sendTo):
   Msg( msgType, afClass),
   next_ptr( NULL),
   address( NULL),
   recieve( toRecieve)
{
   if( sendTo ) address = new af::Address( sendTo);
}

QMsg::~QMsg()
{
   if( address != NULL) delete address;
}
