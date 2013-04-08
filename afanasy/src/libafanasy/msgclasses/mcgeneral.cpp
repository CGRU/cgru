#include "mcgeneral.h"

#include <stdio.h>

#include "../name_af.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../../include/macrooutput.h"

using namespace af;

MCGeneral::MCGeneral():
   name( ""),
   string( ""),
   number( 0),
   id(-1)
{
}

MCGeneral::MCGeneral( int32_t Number):
   name( ""),
   string( ""),
   number( Number),
   id(-1)
{
}

MCGeneral::MCGeneral( const std::string & String):
   name( ""),
   string( String),
   number( 0),
   id(-1)
{
}


MCGeneral::MCGeneral( const std::string & Name, int32_t Number):
   name( Name),
   string( ""),
   number( Number),
   id(-1)
{
}

MCGeneral::MCGeneral( const std::string & Name, const std::string & String):
   name( Name),
   string( String),
   number( 0),
   id(-1)
{
}

MCGeneral::MCGeneral( Msg * msg)
{
   read( msg);
}

MCGeneral::~MCGeneral()
{
}

void MCGeneral::v_readwrite( Msg * msg)
{
   MsgClassUserHost::v_readwrite( msg);

   rw_String(     name,   msg);
   rw_String(     string, msg);
   rw_int32_t(    number, msg);
   rw_int32_t(    id,     msg);
   rw_Int32_Vect( list,   msg);
}

bool MCGeneral::hasId( int value)
{
   int count = int( list.size());
   for( int i = 0; i < count; i++)
      if( list[i] == value) return true;
   return false;
}

void MCGeneral::setList( const std::list<int32_t> & i_list)
{
	list.clear();
	std::list<int32_t>::const_iterator it = i_list.begin();
	while( it != i_list.end())
	{
		list.push_back(*it);
		it++;
	}
}

void MCGeneral::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
   MsgClassUserHost::v_generateInfoStream( stream, false);

   stream << "\nName = \"" << name << "\"" << ", Id = " << id << ", String = \"" << string << "\"" << ", Number = " << number;

   if( full == false ) return;

   int count = int(list.size());
   stream << ", ids[" << count << "]=";
   for( int p = 0; p < count; p++) stream << " " << list[p];
}
