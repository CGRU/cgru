#pragma once

#include "msgclass.h"

#include <QtCore/QStringList>

namespace af
{

class MCTalkdistmessage : public MsgClass
{
public:
   MCTalkdistmessage( const QString &userfrom, const QString &msgtext);
   MCTalkdistmessage( Msg * msg);
   ~MCTalkdistmessage();

   inline void addUser( const QString &str) { if(!list.contains(str)) list<<str;}

   inline void getUser( QString &str) const { str=user; }
   inline void getText( QString &str) const { str=text; }

   inline const QStringList* getList() const { return &list; }

   void stdOut( bool full = false) const;

private:
   QString user;
   QString text;
   QStringList list;
   void readwrite( Msg * msg);
};
}
