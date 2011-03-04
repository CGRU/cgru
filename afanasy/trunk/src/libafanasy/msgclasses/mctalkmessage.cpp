#include "mctalkmessage.h"

#include <stdio.h>

#include "../name_af.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../../include/macrooutput.h"

using namespace af;

MCTalkmessage::MCTalkmessage( const std::string & msguser, const std::string & msgtext)
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
   rw_String( user, msg);
   rw_String( text, msg);
}

void MCTalkmessage::stdOut( bool full) const
{
   printf("%s: %s\n", user.c_str(), text.c_str());
}
