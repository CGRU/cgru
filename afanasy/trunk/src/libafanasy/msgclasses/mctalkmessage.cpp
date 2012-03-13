#include "mctalkmessage.h"

#include <stdio.h>

#include "../name_af.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../../include/macrooutput.h"

using namespace af;

MCTalkmessage::MCTalkmessage( const QString &msguser, const QString &msgtext)
{
   user = msguser;
   text = msgtext;
}

MCTalkmessage::MCTalkmessage( Msg * msg)
{
   read( msg);
}

MCTalkmessage::~MCTalkmessage()
{
}

void MCTalkmessage::readwrite( Msg * msg)
{
   rw_QString( user, msg);
   rw_QString( text, msg);
}

void MCTalkmessage::stdOut( bool full) const
{
   printf("%s: %s\n", user.toUtf8().data(), text.toUtf8().data());
}
