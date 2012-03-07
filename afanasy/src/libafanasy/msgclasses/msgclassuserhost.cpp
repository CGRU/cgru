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

void MsgClassUserHost::generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( full )
   {
      stream << "User = \"" << username << "\", Host = \"" << hostname << "\"";
   }
   else
   {
      stream << username << "@" << hostname;
   }
}
