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

   virtual void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

   inline const std::string & getUserName() const { return username; }
   inline const std::string & getHostName() const { return hostname; }

   inline void setUserName( const std::string & value) { username = value;}
   inline void setHostName( const std::string & value) { hostname = value;}

protected:
   void v_readwrite( Msg * msg);

private:
   std::string username;
   std::string hostname;

private:
};
}
