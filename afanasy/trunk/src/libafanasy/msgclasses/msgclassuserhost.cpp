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
   rw_QString( username, msg);
   rw_QString( hostname, msg);
}

void MsgClassUserHost::stdOut( bool full) const
{
   if( full )
      printf("User = \"%s\", Host = \"%s\"\n", username.toUtf8().data(), hostname.toUtf8().data());
   else                        printf("%s@%s", username.toUtf8().data(), hostname.toUtf8().data());
}
