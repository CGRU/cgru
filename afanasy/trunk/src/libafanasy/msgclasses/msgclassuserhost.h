#pragma once

#include <QtCore/QString>

#include "msgclass.h"

namespace af
{

class MsgClassUserHost : public MsgClass
{
public:
   MsgClassUserHost();
   MsgClassUserHost( Msg * msg);

   virtual ~MsgClassUserHost();

   void stdOut( bool full = false) const;

   inline const QString & getUserName() const { return username; }
   inline const QString & getHostName() const { return hostname; }

protected:
   void readwrite( Msg * msg);

private:
   QString username;
   QString hostname;

private:
};
}
