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

void MsgClassUserHost::v_readwrite( Msg * msg)
{
   rw_String( username, msg);
   rw_String( hostname, msg);
}

void MsgClassUserHost::v_generateInfoStream( std::ostringstream & stream, bool full) const
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
