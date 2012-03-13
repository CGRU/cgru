#include "mctalkdistmessage.h"

#include <stdio.h>

#include <name_af.h>

//#define AFOUTPUT
////#undef AFOUTPUT
#include <macrooutput.h>

using namespace af;

MCTalkdistmessage::MCTalkdistmessage( const QString &userfrom, const QString &msgtext)
{
   user = userfrom;
   text = msgtext;
}

MCTalkdistmessage::MCTalkdistmessage( Msg * msg)
{
   read( msg);
}

MCTalkdistmessage::~MCTalkdistmessage()
{
}

void MCTalkdistmessage::readwrite( Msg * msg)
{
   rw_QString(     user, msg);
   rw_QString(     text, msg);
   rw_QStringList( list, msg);
}

void MCTalkdistmessage::stdOut( bool full) const
{
   printf("%s: %s\n", user.toUtf8().data(), text.toUtf8().data());
}
