#pragma once

#include "msgclass.h"

namespace af
{

class MCTalkdistmessage : public MsgClass
{
public:
   MCTalkdistmessage( const std::string & userfrom, const std::string & msgtext);
   MCTalkdistmessage( Msg * msg);
   ~MCTalkdistmessage();

   inline void addUser( const std::string & str) { list.push_back( str); list.unique();}

   inline void getUser( std::string & str) const { str = user; }
   inline void getText( std::string & str) const { str = text; }

   inline const std::list<std::string> * getList() const { return &list; }

   void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

private:
   std::string user;
   std::string text;
   std::list<std::string> list;
   void v_readwrite( Msg * msg);
};
}
