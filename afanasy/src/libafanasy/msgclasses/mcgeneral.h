#pragma once

#include <stdlib.h>
#include <vector>

#include "msgclassuserhost.h"

namespace af
{

class MCGeneral : public MsgClassUserHost
{
public:
   MCGeneral();
   MCGeneral( const std::string & String);
   MCGeneral( int Number);
   MCGeneral( const std::string & Name, int Number);
   MCGeneral( const std::string & Name, const std::string & String);

   MCGeneral( Msg * msg);
   virtual ~MCGeneral();

   inline void setNumber( int Number) { number = Number;}
   inline void setString( const std::string & String) { string = String;}

   inline void addId( int ID) { list.push_back( ID);}
   inline void setId( int ID) { id = ID;}

   virtual void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

   inline const std::string & getName()   const { return name;   }
   inline const std::string & getString() const { return string; }

   inline size_t getCount() const { return list.size(); }
   inline int getId( unsigned pos) const { if(pos<list.size())return list[pos]; else return -1; }
   inline int getId() const { return id; }
   bool hasId( int value);

   inline int getNumber() const { return number; }

   inline const std::vector<int32_t> & getList() const { return list; }

	void setList( const std::list<int32_t> & i_list);
	inline void setList( const std::vector<int32_t> & i_list) { list = i_list;}

   inline void clearIds() { list.clear(); }

private:
   std::string name;
   std::string string;
   int32_t number;
   int32_t id;

   std::vector<int32_t> list;

   void v_readwrite( Msg * msg);
};
}
