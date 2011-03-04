#pragma once

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

   inline const std::string & getUserName() const { return username; }
   inline const std::string & getHostName() const { return hostname; }

protected:
   void readwrite( Msg * msg);

private:
   std::string username;
   std::string hostname;

private:
};
}
