#pragma once

#include "msgclass.h"

#include <QtCore/QString>

namespace af
{

class MCTalkmessage : public MsgClass
{
public:
   MCTalkmessage( const QString &msguser, const QString &msgtext);
   MCTalkmessage( Msg * msg);
   ~MCTalkmessage();

   inline void getUser( QString &str) const { str=user; }
   inline void getText( QString &str) const { str=text; }

   void stdOut( bool full = false) const;

private:
   QString user;
   QString text;
   void readwrite( Msg * msg);
};
}
