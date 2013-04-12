#include "af.h"

#include "msg.h"
#include "regexp.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

Af::Af()  {};
Af::~Af() {};

/* Solution from:
http://stackoverflow.com/questions/3022552/is-there-any-standard-htonl-like-function-for-64-bits-integers-in-c
( 2011-05-31 )*/
int64_t swap64( int64_t value)
{
	 // The answer is 42
	 static const int num = 42;

	 // Check the endianness
	 if (*reinterpret_cast<const char*>(&num) == num)
	 {
		  const uint32_t high_part = htonl(static_cast<uint32_t>(value >> 32));
		  const uint32_t low_part = htonl(static_cast<uint32_t>(value & 0xFFFFFFFFLL));

		  return (static_cast<int64_t>(low_part) << 32) | high_part;
	 } else
	 {
		  return value;
	 }
}

void Af::write( Msg * msg )
{
	v_readwrite( msg);
}

void Af::read( Msg * msg )
{
	v_readwrite( msg);
}

void Af::v_readwrite( Msg * msg )
{
	AFERROR("Af::readwrite: Not implemented.\n")
}

void Af::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
	stream << "\nAf::generateInfoStream\n";
	AFERROR("Af::generateInfoStream: Not implemented.\n")
}

const std::string Af::v_generateInfoString( bool full) const
{
	std::ostringstream stream;
	v_generateInfoStream( stream, full);
	return stream.str();
}

void Af::v_stdOut( bool full) const
{
	std::cout << v_generateInfoString( full) << std::endl;
}

void Af::rw_data( char* data, Msg * msg, int size)
{
	char * buffer = msg->writtenBuffer( size);
	if( buffer == NULL) return;

	if( msg->isWriting() )  memcpy( buffer, data,   size);
	else                    memcpy( data,   buffer, size);
}

void Af::w_data( const char * data, Msg * msg, int size)
{
	char * buffer = msg->writtenBuffer( size);
	if( buffer == NULL) return;
	if( msg->isWriting()) memcpy( buffer, data, size);
	else AFERROR("Af::w_data: Message is not for reading.\n")
}

void Af::rw_bool( bool& boolean, Msg * msg)
{
	const int size = 1;
	char * buffer = msg->writtenBuffer( size);
	if( buffer == NULL) return;

	if( msg->isWriting() )
	{
		if( boolean) *buffer = 'y';
		else *buffer = 'n';
	}
	else
	{
		if( *buffer == 'y') boolean = true;
		else boolean = false;
	}
}

void Af::rw_uint8_t( uint8_t& integer, Msg * msg)
{
	const int size = 1;
	char * buffer = msg->writtenBuffer( size);
	if( buffer == NULL) return;

	if( msg->isWriting() )  memcpy( buffer,   &integer, size);
	else                    memcpy( &integer, buffer,   size);
//printf("Af::rw_uint8_t: integer = %u (buffer=%p)\n", integer, buffer);
}

void Af::rw_uint16_t( uint16_t& integer, Msg * msg)
{
	const int size = 2;
	char * buffer = msg->writtenBuffer( size);
	if( buffer == NULL) return;

	uint16_t bytes;
	if( msg->isWriting() )
	{
		bytes = htons( integer);
		memcpy( buffer, &bytes, size);
	}
	else
	{
		memcpy( &bytes, buffer, size);
		integer = ntohs( bytes);
	}
}

void Af::rw_uint32_t( uint32_t& integer, Msg * msg)
{
	const int size = 4;
	char * buffer = msg->writtenBuffer( size);
	if( buffer == NULL) return;

	uint32_t bytes;
	if( msg->isWriting() )
	{
		bytes = htonl( integer);
		memcpy( buffer, &bytes, size);
	}
	else
	{
		memcpy( &bytes, buffer, size);
		integer = ntohl( bytes);
	}
//printf("Af::rw_uint32_t: integer = %d (buffer=%p)\n", integer, buffer);
}

void Af::rw_int8_t( int8_t& integer, Msg * msg)
{
	const int size = 1;
	char * buffer = msg->writtenBuffer( size);
	if( buffer == NULL) return;

	if( msg->isWriting() )  memcpy( buffer,   &integer, size);
	else                    memcpy( &integer, buffer,   size);
}

void Af::rw_int16_t( int16_t& integer, Msg * msg)
{
	const int size = 2;
	char * buffer = msg->writtenBuffer( size);
	if( buffer == NULL) return;

	int16_t bytes;
	if( msg->isWriting() )
	{
		bytes = htons( integer);
		memcpy( buffer, &bytes, size);
	}
	else
	{
		memcpy( &bytes, buffer, size);
		integer = ntohs( bytes);
	}
}

void Af::rw_int32_t( int32_t& integer, Msg * msg)
{
	const int size = 4;
	char * buffer = msg->writtenBuffer( size);
	if( buffer == NULL) return;

	int32_t bytes;
	if( msg->isWriting() )
	{
		bytes = htonl( integer);
		memcpy( buffer, &bytes, size);
	}
	else
	{
		memcpy( &bytes, buffer, size);
		integer = ntohl( bytes);
	}
}

void Af::rw_int64_t( int64_t  & integer, Msg * msg)
{
	const int size = 8;
	char * buffer = msg->writtenBuffer( size);
	if( buffer == NULL) return;

	int64_t bytes;
	if( msg->isWriting() )
	{
		bytes = swap64( integer);
		memcpy( buffer, &bytes, size);
	}
	else
	{
		memcpy( &bytes, buffer, size);
		integer = swap64( bytes);
	}
}

void Af::w_String( const std::string & string, Msg * msg)
{
	if( false == msg->isWriting())
	{
		AFERROR("Af::w_String: Message is not for reading.\n")
		return;
	}
	const char * buffer = string.c_str();
	uint32_t length = uint32_t( string.length()) + 1;
	rw_uint32_t( length, msg);
	w_data( buffer, msg, length);
//std::cout << "Af::w_String: string = \"" << string << "\"\n";
}

void Af::rw_String( std::string & string, Msg * msg)
{
	uint32_t length;

	if( msg->isWriting())
	{
		const char * buffer = string.c_str();
		length = uint32_t( string.length()) + 1;
		rw_uint32_t( length, msg);
		w_data( buffer, msg, length);
	}
	else
	{
		rw_uint32_t( length, msg);
		char * buffer = msg->writtenBuffer( length);
		if( buffer == NULL ) return;
		string = std::string(buffer, length-1);
	}
//std::cout << "Af::rw_String: string = \"" << string << "\"\n";
}

void Af::rw_RegExp( RegExp & regExp, Msg * msg)
{
	if( msg->isWriting())
	{
		w_String( regExp.getPattern(), msg);
	}
	else
	{
		std::string pattern;
		rw_String( pattern, msg);
		regExp.setPattern( pattern);
	}
}

void Af::rw_StringVect( std::vector<std::string> & stringVect, Msg * msg)
{
	uint32_t length;
	if( msg->isWriting() ) length = stringVect.size();
	rw_uint32_t( length, msg);
	if( msg->isWriting() )
		for( std::vector<std::string>::iterator it = stringVect.begin(); it != stringVect.end(); it++)
			rw_String( *it, msg);
	else
		for( unsigned i = 0; i < length; i++)
		{
			std::string str;
			rw_String( str, msg);
			stringVect.push_back( str);
		}
}

void Af::rw_StringList( std::list<std::string> & stringList, Msg * msg)
{
	uint32_t length;
	if( msg->isWriting() ) length = uint32_t(stringList.size());
	rw_uint32_t( length, msg);
	if( msg->isWriting() )
		for( std::list<std::string>::iterator it = stringList.begin(); it != stringList.end(); it++)
			rw_String( *it, msg);
	else
		for( unsigned i = 0; i < length; i++)
		{
			std::string str;
			rw_String( str, msg);
			stringList.push_back( str);
		}
//std::cout << "rw_StringList: length = \"" << length << "\"\n";
}

void Af::w_StringList( const std::list<std::string> & stringList, Msg * msg)
{
	if( false == msg->isWriting())
	{
		AFERROR("Af::w_StringList: Message is not for reading.\n")
		return;
	}

	uint32_t length = uint32_t(stringList.size());
	rw_uint32_t( length, msg);
	for( std::list<std::string>::const_iterator it = stringList.begin(); it != stringList.end(); it++)
	{
		w_String( *it, msg);
	}
//std::cout << "w_StringList: length = \"" << length << "\"\n";
}

void Af::rw_Int32_List( std::list<int32_t> &list, Msg * msg)
{
	uint32_t count;
	if( msg->isWriting())
	{
		count = uint32_t( list.size());
		rw_uint32_t( count, msg);
		if( count < 1 ) return;
		std::list<int32_t>::iterator it = list.begin();
		std::list<int32_t>::iterator end_it = list.end();
		while( it != end_it) rw_int32_t( *(it++), msg);
	}
	else
	{
		rw_uint32_t( count, msg);
		for( unsigned p = 0; p < count; p++)
		{
			int32_t integer;
			rw_int32_t( integer, msg);
			list.push_back( integer);
		}
	}
}

void Af::rw_Int32_Vect( std::vector<int32_t> &vect, Msg * msg)
{
	uint32_t count;
	if( msg->isWriting() )
	{
		count = uint32_t( vect.size());
		rw_uint32_t( count, msg);
		for( unsigned p = 0; p < count; p++) rw_int32_t( vect[p], msg);
	}
	else
	{
		rw_uint32_t( count, msg);
		int32_t integer;
		for( unsigned p = 0; p < count; p++)
		{
			rw_int32_t( integer, msg);
			vect.push_back( integer);
		}
	}
}

