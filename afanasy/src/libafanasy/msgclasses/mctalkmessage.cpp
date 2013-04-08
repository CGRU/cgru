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

void MCTalkmessage::v_readwrite( Msg * msg)
{
   rw_String( user, msg);
   rw_String( text, msg);
}

void MCTalkmessage::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
   stream << user << ": " << text;
}
