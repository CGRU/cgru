#include "msgclassuserhost.h"

#include <stdio.h>

#include "../environment.h"

using namespace af;

MsgClassUserHost::MsgClassUserHost():
   username( af::Environment::getUserName()),
   hostname( af::Environment::getHostName())
{};

MsgClassUserHost::MsgClassUserHost( Msg * msg)
{
   read( msg);
}

MsgClassUserHost::~MsgClassUserHost(){};

void MsgClassUserHost::readwrite( Msg * msg)
{
   rw_String( username, msg);
   rw_String( hostname, msg);
}

void MsgClassUserHost::stdOut( bool full) const
{
   if( full )
      printf("User = \"%s\", Host = \"%s\"\n", username.c_str(), hostname.c_str());
   else                        printf("%s@%s", username.c_str(), hostname.c_str());
}
