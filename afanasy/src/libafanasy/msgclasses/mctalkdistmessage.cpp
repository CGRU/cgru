#include "mctalkdistmessage.h"

#include <stdio.h>

#include "../name_af.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../../include/macrooutput.h"

using namespace af;

MCTalkdistmessage::MCTalkdistmessage( const std::string & userfrom, const std::string & msgtext):
   user( userfrom),
   text( msgtext)
{
}

MCTalkdistmessage::MCTalkdistmessage( Msg * msg)
{
   read( msg);
}

MCTalkdistmessage::~MCTalkdistmessage()
{
}

void MCTalkdistmessage::v_readwrite( Msg * msg)
{
   rw_String(     user, msg);
   rw_String(     text, msg);
   rw_StringList( list, msg);
}

void MCTalkdistmessage::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
   stream << user << ": " << text;
}
