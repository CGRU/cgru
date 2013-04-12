#pragma once

#ifndef WINNT
#include <netinet/in.h>
#include <netdb.h>
#else
#include <winsock2.h>
#endif

#include "rapidjson/document.h"

#include "name_af.h"

namespace af
{

class Af
{
public:
	Af();
	virtual ~Af();

	void write( Msg * msg );

	virtual void v_stdOut( bool full = false ) const;
	virtual const std::string v_generateInfoString( bool full = false) const;
	virtual void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;


protected:
	void read( Msg * msg );
	virtual void v_readwrite( Msg * msg );

	static void rw_bool     ( bool     & boolean,  Msg * msg);
	static void rw_int8_t   ( int8_t   & integer,  Msg * msg);
	static void rw_uint8_t  ( uint8_t  & integer,  Msg * msg);
	static void rw_int16_t  ( int16_t  & integer,  Msg * msg);
	static void rw_uint16_t ( uint16_t & integer,  Msg * msg);
	static void rw_int32_t  ( int32_t  & integer,  Msg * msg);
	static void rw_uint32_t ( uint32_t & integer,  Msg * msg);
	static void rw_int64_t  ( int64_t  & integer,  Msg * msg);

	static void rw_Int32_List( std::list   < int32_t > &list, Msg * msg);
	static void rw_Int32_Vect( std::vector < int32_t > &vect, Msg * msg);

	static void rw_String(       std::string & string, Msg * msg);
	static void  w_String( const std::string & string, Msg * msg);

	static void rw_StringList(       std::list<std::string> & stringList, Msg * msg);
	static void  w_StringList( const std::list<std::string> & stringList, Msg * msg);
	static void rw_StringVect(     std::vector<std::string> & stringVect, Msg * msg);

	static void rw_RegExp( RegExp & regExp, Msg * msg);

	static void rw_data(       char * data, Msg * msg, int size);
	static void  w_data( const char * data, Msg * msg, int size);
};
}
