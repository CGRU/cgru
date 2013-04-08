#pragma once

#include "msgclass.h"

namespace af
{

class MCTalkmessage : public MsgClass
{
public:
   MCTalkmessage( const std::string & msguser, const std::string & msgtext);
   MCTalkmessage( Msg * msg);
   ~MCTalkmessage();

   inline void getUser( std::string & str) const { str=user; }
   inline void getText( std::string & str) const { str=text; }

   void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

private:
   std::string user;
   std::string text;
   void v_readwrite( Msg * msg);
};
}
