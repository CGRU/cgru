#include "name_af.h"

#ifndef WINNT
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#define closesocket close
#endif

#include "../include/afanasy.h"

#include "address.h"
#include "environment.h"
#include "msg.h"
#include "msgstat.h"
#include "regexp.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

af::MsgStat mgstat;

/// Read data from file descriptor. Return bytes than was written or -1 on any error and prints an error in \c stderr.
int readdata( int fd, char* data, int data_len, int buffer_maxlen)
{
	AFINFA("readdata: trying to receive %d bytes.\n", data_len);
	int bytes = 0;
	while( bytes < data_len )
	{
#ifdef WINNT
		int r = recv( fd, data+bytes, buffer_maxlen-bytes, 0);
#else
		int r = read( fd, data+bytes, buffer_maxlen-bytes);
#endif
		if( r < 0)
		{
			AFERRPE("readdata: read");
			return -1;
		}
		AFINFA("readdata: read %d bytes.\n", r);
		if( r == 0) return bytes;
		bytes += r;
	}

	return bytes;
}

/// Write data to file descriptor. Return \c false on any arror and prints an error in \c stderr.
bool writedata( int fd, const char * data, int len)
{
	int written_bytes = 0;
	while( written_bytes < len)
	{
#ifdef WINNT
		int w = send( fd, data+written_bytes, len, 0);
#else
		int w = write( fd, data+written_bytes, len);
#endif
		if( w < 0)
		{
			AFERRPE("name_afnet.cpp writedata:");
			return false;
		}
		written_bytes += w;
	}
	return true;
}

// Return header offset or -1, if it was not recognized.
int af::processHeader( af::Msg * io_msg, int i_bytes)
{
	//printf("name_afnet.cpp processHeader: Received %d bytes:\n", i_bytes);
	char * buffer = io_msg->buffer();
	int offset = 0;

	// Process HTTP header:
	if( strncmp( buffer, "POST", 4) == 0 )
	{
		//writedata( 1, buffer, i_bytes); write(1,"\n",1);
		offset = 4;
		int size;
		bool header_processed = false;
		for( offset = 4; offset < i_bytes; offset++)
		{
			// Look for line end:
			if( buffer[offset] == '\n' )
			{
				// Go to line begin:
				offset++;
				if( offset == i_bytes )
					break;

				// If header found, body can start in the same data packet:
				if( header_processed && ( buffer[offset] == '{' ))
				{
					//write(1,"\nBODY FOUND:\n", 15);
					//write(1, buffer+offset, i_bytes - offset);
					//write(1,"\n",1);
					break;
				}

				// Look for a special header:
				if( strncmp("AFANASY: ", buffer+offset, 9) == 0)
				{
					//printf("\nAFANASY FOUND:\n");
					offset += 9;
					if( 1 == sscanf( buffer + offset, "%d", &size))
					{
						//printf("\nHEADER FOUND: size=%d\n", size);
						header_processed = true;
					}
					else
					{
						AFERROR("HTTP POST request has a bad AFANASY header.")
						return -1;
					}
				}
			}
		}

		// If header found, construct message:
		if( header_processed )
		{
			io_msg->setHeader( af::Msg::THTTP, size, offset, i_bytes);
			return offset;
		}

		// Header not recongnized:
		AFERROR("HTTP POST request was not recongnized.")
		return -1;
	}

	// Simple header for JSON (used for example in python api and afcmd)
	if( strncmp("AFANASY", buffer, 7) == 0 )
	{
		//writedata( 1, buffer+offset, i_bytes);printf("\n");
		offset += 7;
		int size;
		int num = sscanf( buffer + offset, "%d", &size);
		//printf("\n sscanf=%d\n",num);
		if( num == 1 )
		{
			while( ++offset < i_bytes )
			{
				if( strncmp( buffer+offset, "JSON", 4) == 0)
				{
					offset += 4;
					while( offset < i_bytes )
					{
						if( buffer[offset] == '{' )
						{
							break;
						}
						else
						{
							offset++;
							//printf("FOUND: size=%d Offset=%d:\n", size, offset);
							//write(1, buffer, offset);
							//write(1, buffer+offset, i_bytes - offset);
							//write(1,"\n",1);
//							io_msg->setHeader( af::Msg::TJSON, size, offset, i_bytes);
							//return false;
							//io_msg->stdOutData();
//							return offset;
						}
					}
					io_msg->setHeader( af::Msg::TJSON, size, offset, i_bytes);
					return offset;
				}
			}

			// Header not recongnized:
			AFERROR("JSON message header was not recongnized.")
			return -1;
		}
	}

	if( strncmp( buffer, "GET", 3) == 0 )
	{
		//writedata( 1, buffer, i_bytes);
		char * get = new char[i_bytes];
		memcpy( get, buffer, i_bytes);
		io_msg->setData( i_bytes, get, af::Msg::THTTPGET);
		delete []  get;

		//printf("i_bytes = %d, msg data len = %d\n", i_bytes, io_msg->dataLen());
		return 0; // no offset, reading finished
	}

	io_msg->readHeader( i_bytes);

	return af::Msg::SizeHeader;
}

af::Msg * msgsendtoaddress( const af::Msg * i_msg, const af::Address & i_address,
						    bool & o_ok, af::VerboseMode i_verbose)
{
	if( af::Environment::isServer())
	{
		AFERROR("msgsendtoaddress: Server should not connect and send messages itself.\n")
		o_ok = false;
		return NULL;
	}

	o_ok = true;

	if( i_address.isEmpty() )
	{
		AFERROR("msgsendtoaddress: Address is empty.")
		o_ok = false;
		return NULL;
	}

	int socketfd;
	struct sockaddr_storage client_addr;

	if( false == i_address.setSocketAddress( &client_addr)) return NULL;

	if(( socketfd = socket( client_addr.ss_family, SOCK_STREAM, 0)) < 0 )
	{
		AFERRPE("msgsendtoaddress: socket() call failed")
		o_ok = false;
		return NULL;
	}


	// Set socket options:
	af::setSocketOptions( socketfd);


	//
	// connect to address
	AFINFO("msgsendtoaddress: tying to connect to client.")
	if( connect(socketfd, (struct sockaddr*)&client_addr, i_address.sizeofAddr()) != 0 )
	{
		if( i_verbose == af::VerboseOn )
		{
			AFERRPA("msgsendtoaddress: connect failure for msgType '%s':\n%s: ",
				af::Msg::TNAMES[i_msg->type()], i_address.v_generateInfoString().c_str())
		}
		closesocket(socketfd);
		o_ok = false;
		return NULL;
	}


	//
	// send
	if( false == af::msgwrite( socketfd, i_msg))
	{
		AFERRAR("af::msgsendtoaddress: can't send message to client: %s",
				i_address.v_generateInfoString().c_str())

		closesocket( socketfd);

		o_ok = false;
		return NULL;
	}


	// Read JSON answer:
	if( i_msg->type() == af::Msg::TJSON )
	{
		static const int read_buf_len = 4096;
		char read_buf[read_buf_len];
		std::string buffer;
		while( buffer.size() <= af::Msg::SizeDataMax )
		{
			#ifdef WINNT
			int r = recv( socketfd, read_buf, read_buf_len, 0);
			#else
			int r = read( socketfd, read_buf, read_buf_len);
			#endif
			if( r <= 0 )
				break;
			buffer += std::string( read_buf, r);
		}

		af::Msg * o_msg = NULL;
		if( buffer.size())
		{
			o_msg = new af::Msg();
			o_msg->setData( buffer.size(), buffer.c_str(), af::Msg::TJSON);
			o_ok = true;
		}
		else
		{
			AFERROR("msgsendtoaddress: Reading JSON answer failed.")
			o_ok = false;
		}

		closesocket( socketfd);
		return o_msg;
	}

	//
	// Read binary answer:
	af::Msg * o_msg = new af::Msg();
	if( false == af::msgread( socketfd, o_msg))
	{
	   AFERROR("msgsendtoaddress: Reading binary answer failed.")
		closesocket( socketfd);
	   delete o_msg;
	   o_ok = false;
	   return NULL;
	}

	closesocket( socketfd);

	return o_msg;
}

void af::statwrite( af::Msg * msg)
{
   mgstat.writeStat( msg);
}
void af::statread( af::Msg * msg)
{
   mgstat.readStat( msg);
}

void af::statout( int columns, int sorting)
{
   mgstat.v_stdOut( columns, sorting);
}

const af::Address af::solveNetName( const std::string & i_name, int i_port, int i_type, VerboseMode i_verbose)
{
	if( i_verbose == af::VerboseOn )
	{
		printf("Solving '%s'", i_name.c_str());
		switch( i_type)
		{
			case AF_UNSPEC: break;
			case AF_INET:  printf(" and IPv4 forced"); break;
			case AF_INET6: printf(" and IPv6 forced"); break;
			default: printf(" (unknown protocol forced)");
		}
		printf("...\n");
	}

	struct addrinfo *res;
	struct addrinfo hints;
	memset( &hints, 0, sizeof(hints));
	hints.ai_flags = AI_ADDRCONFIG;
 //   hints.ai_family = AF_UNSPEC; // This is value is default
	hints.ai_socktype = SOCK_STREAM;
	char service_port[16];
	sprintf( service_port, "%u", i_port);
	int err = getaddrinfo( i_name.c_str(), service_port, &hints, &res);
	if( err != 0 )
	{
		AFERRAR("af::solveNetName:\n%s", gai_strerror(err))
		return af::Address();
	}

	for( struct addrinfo *r = res; r != NULL; r = r->ai_next)
	{
		// Skip address if type is forced
		if(( i_type != AF_UNSPEC ) && ( i_type != r->ai_family)) continue;

		if( i_verbose == af::VerboseOn )
		{
			switch( r->ai_family)
			{
				case AF_INET:
				{
					struct sockaddr_in * sa = (struct sockaddr_in*)(r->ai_addr);
					printf("IP = %s\n", inet_ntoa( sa->sin_addr));
					break;
				}
				case AF_INET6:
				{
					static const int buffer_len = 256;
					char buffer[buffer_len];
					struct sockaddr_in6 * sa = (struct sockaddr_in6*)(r->ai_addr);
					const char * addr_str = inet_ntop( AF_INET6, &(sa->sin6_addr), buffer, buffer_len);
					printf("IPv6 = %s\n", addr_str);
					break;
				}
				default:
					printf("Unknown address family type = %d\n", r->ai_family);
					continue;
			}
		}

		af::Address addr((struct sockaddr_storage*)(r->ai_addr));

		if( i_verbose == af::VerboseOn )
		{
			printf("Address = ");
			addr.v_stdOut();
		}

		// Free memory allocated for addresses:
		freeaddrinfo( res);

		return addr;

	}

	// Free memory allocated for addresses:
	freeaddrinfo( res);

	return af::Address();
}

void af::setSocketOptions( int i_fd)
{
	if( af::Environment::getSO_REUSEADDR() != -1 )
	{
		int reuseaddr = af::Environment::getSO_REUSEADDR();
		if( setsockopt( i_fd, SOL_SOCKET, SO_REUSEADDR, WINNT_TOCHAR(&reuseaddr), sizeof(reuseaddr)) != 0)
			AFERRPE("Set socket SO_REUSEADDR option failed:")
	}

	if( af::Environment::getSO_RCVTIMEO_sec() != -1 )
	{
		#ifdef WINNT
		DWORD so_timeo = 1000 * af::Environment::getSO_RCVTIMEO_sec();
		#else
		timeval so_timeo;
		so_timeo.tv_usec = 0;
		so_timeo.tv_sec = af::Environment::getSO_RCVTIMEO_sec();
		#endif
		if( setsockopt( i_fd, SOL_SOCKET, SO_RCVTIMEO, WINNT_TOCHAR(&so_timeo), sizeof(so_timeo)) != 0)
			AFERRPE("Set socket SO_RCVTIMEO option failed:")
	}

	if( af::Environment::getSO_SNDTIMEO_sec() != -1 )
	{
		#ifdef WINNT
		DWORD so_timeo = 1000 * af::Environment::getSO_SNDTIMEO_sec();
		#else
		timeval so_timeo;
		so_timeo.tv_usec = 0;
		so_timeo.tv_sec = af::Environment::getSO_SNDTIMEO_sec();
		#endif
		if( setsockopt( i_fd, SOL_SOCKET, SO_SNDTIMEO, WINNT_TOCHAR(&so_timeo), sizeof(so_timeo)) != 0)
			AFERRPE("Set socket SO_SNDTIMEO option failed:")
	}

	if( af::Environment::getSO_LINGER() != -1 )
	{
		linger so_linger;
		so_linger.l_onoff  = af::Environment::getSO_LINGER();
		so_linger.l_linger = af::Environment::getSO_LINGER();
		if( setsockopt( i_fd, SOL_SOCKET, SO_LINGER, WINNT_TOCHAR(&so_linger), sizeof(so_linger)) != 0)
			AFERRPE("Set socket SO_LINGER option failed:")
	}

	if( af::Environment::getSO_TCP_NODELAY() != -1 )
	{
		int nodelay = af::Environment::getSO_TCP_NODELAY();
		if( setsockopt( i_fd, IPPROTO_TCP, TCP_NODELAY, WINNT_TOCHAR(&nodelay), sizeof(nodelay)) != 0)
			AFERRPE("Set socket TCP_NODELAY option failed:")
	}
	#ifdef LINUX
	if( af::Environment::getSO_TCP_CORK() != -1 )
	{
		int cork = af::Environment::getSO_TCP_CORK();
		if( setsockopt( i_fd, SOL_TCP, TCP_CORK, &cork, sizeof(cork)) != 0)
			AFERRPE("Set socket TCP_CORK option failed:")
	}
	#endif
}

bool af::msgread( int desc, af::Msg* msg)
{
AFINFO("af::msgread:\n");

	char * buffer = msg->buffer();
	//
	// Read message header data
	int bytes = ::readdata( desc, buffer, af::Msg::SizeHeader, af::Msg::SizeBuffer );

	if( bytes < af::Msg::SizeHeader)
	{
		AFERRAR("af::msgread: can't read message header, bytes = %d (< Msg::SizeHeader).", bytes)
		msg->setInvalid();
		return false;
	}

	// Header offset is variable on not binary header (for example HTTP)
	int header_offset = af::processHeader( msg, bytes);
	if( header_offset < 0)
		return false;

	//
	// Read message data if any
	if( msg->type() >= af::Msg::TDATA)
	{
		buffer = msg->buffer(); // buffer may be changed to fit new size
		bytes -= header_offset;
		int readlen = msg->dataLen() - bytes;
		if( readlen > 0)
		{
//printf("Need to read more %d bytes of data:\n", readlen);
			bytes = ::readdata( desc, buffer + af::Msg::SizeHeader + bytes, readlen, readlen);
			if( bytes < readlen)
			{
				AFERRAR("af::msgread: read message data: ( bytes < readlen : %d < %d)", bytes, readlen)
				msg->setInvalid();
				return false;
			}
		}
	}
//msg->stdOutData();
	mgstat.put( msg->type(), msg->writeSize());

	return true;
}

std::string af::msgMakeWriteHeader( const af::Msg * i_msg)
{
	int size = i_msg->writeSize() - i_msg->getHeaderOffset();
	std::string header;
	if( i_msg->type() == af::Msg::THTTP )
	{
		header = af::getHttpHeader(size, "application/json", "200 OK");
	}
	else if( i_msg->type() == af::Msg::TJSON )
	{
		header = std::string("AFANASY ") + af::itos(size) + " JSON";
	}
	return header;
}

std::string af::getHttpHeader(int file_size, const std::string &mimeType, const std::string &status)
{
	int maxAge = 0;
	if (mimeType == "text/css" || mimeType == "text/javascript" || mimeType.find("image/") != -1)
		maxAge = 60 * 60; // enable caching for all static resources (css, js, images)

	return "HTTP/1.1 " + status + "\r\n"  // set the http status code
			 + "Connection: close" + "\r\n"   // instruct browser to close the connection after receiving data
			 + "Content-Length: " + af::itos(file_size) + "\r\n"         // tell how long the content is
			 + "Content-Type: " + mimeType + "\r\n"                            // set the mime type of the result
			 + "Cache-Control: max-age=" + af::itos(maxAge) + "\r\n"     // optional browser caching
			 + "Server: afanasy/" + af::Environment::getVersionCGRU() + "\r\n" // identify server
			 + "\r\n";
}

bool af::msgwrite( int i_desc, const af::Msg * i_msg)
{
	std::string header = af::msgMakeWriteHeader( i_msg);
	if( NULL != header.c_str() )
	{
		::writedata( i_desc, header.c_str(), header.size());
	}

	if( false == ::writedata( i_desc, i_msg->buffer() + i_msg->getHeaderOffset(), i_msg->writeSize() - i_msg->getHeaderOffset() ))
	{
		AFERROR("af::msgwrite: Error writing message.")
		return false;
	}

	mgstat.put( i_msg->type(), i_msg->writeSize());

	return true;
}

af::Msg * af::sendToServer( Msg * i_msg, bool & o_ok, VerboseMode i_verbose )
{
	return ::msgsendtoaddress( i_msg, af::Environment::getServerAddress(), o_ok, i_verbose);
}

af::Msg * af::msgString( const std::string & i_str)
{
	af::Msg * o_msg = new af::Msg();
	o_msg->setString( i_str);
	return o_msg;
}

af::Msg * af::msgInfo( const std::string & i_kind, const std::string & i_info)
{
	af::Msg * o_msg = new af::Msg();
	o_msg->setInfo( i_kind, i_info);
	return o_msg;
}

/*###########################################################################*/
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

void af::rw_data( char* data, Msg * msg, int size)
{
	char * buffer = msg->writtenBuffer( size);
	if( buffer == NULL) return;

	if( msg->isWriting() )  memcpy( buffer, data,   size);
	else                    memcpy( data,   buffer, size);
}

void af::w_data( const char * data, Msg * msg, int size)
{
	char * buffer = msg->writtenBuffer( size);
	if( buffer == NULL) return;
	if( msg->isWriting()) memcpy( buffer, data, size);
	else AFERROR("af::w_data: Message is not for reading.\n")
}

void af::rw_bool( bool& boolean, Msg * msg)
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

void af::rw_uint8_t( uint8_t& integer, Msg * msg)
{
	const int size = 1;
	char * buffer = msg->writtenBuffer( size);
	if( buffer == NULL) return;

	if( msg->isWriting() )  memcpy( buffer,   &integer, size);
	else                    memcpy( &integer, buffer,   size);
//printf("af::rw_uint8_t: integer = %u (buffer=%p)\n", integer, buffer);
}

void af::rw_uint16_t( uint16_t& integer, Msg * msg)
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

void af::rw_uint32_t( uint32_t& integer, Msg * msg)
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
//printf("af::rw_uint32_t: integer = %d (buffer=%p)\n", integer, buffer);
}

void af::rw_int8_t( int8_t& integer, Msg * msg)
{
	const int size = 1;
	char * buffer = msg->writtenBuffer( size);
	if( buffer == NULL) return;

	if( msg->isWriting() )  memcpy( buffer,   &integer, size);
	else                    memcpy( &integer, buffer,   size);
}

void af::rw_int16_t( int16_t& integer, Msg * msg)
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

void af::rw_int32_t( int32_t& integer, Msg * msg)
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

void af::rw_int64_t( int64_t  & integer, Msg * msg)
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

void af::w_String( const std::string & string, Msg * msg)
{
	if( false == msg->isWriting())
	{
		AFERROR("af::w_String: Message is not for reading.\n")
		return;
	}
	const char * buffer = string.c_str();
	uint32_t length = uint32_t( string.length()) + 1;
	rw_uint32_t( length, msg);
	w_data( buffer, msg, length);
//std::cout << "af::w_String: string = \"" << string << "\"\n";
}

void af::rw_String( std::string & string, Msg * msg)
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
//std::cout << "af::rw_String: string = \"" << string << "\"\n";
}

void af::rw_RegExp( RegExp & regExp, Msg * msg)
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

void af::rw_StringVect( std::vector<std::string> & stringVect, Msg * msg)
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

void af::rw_StringList( std::list<std::string> & stringList, Msg * msg)
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

void af::w_StringList( const std::list<std::string> & stringList, Msg * msg)
{
	if( false == msg->isWriting())
	{
		AFERROR("af::w_StringList: Message is not for reading.\n")
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

void af::rw_StringMap( std::map< std::string, std::string > & stringMap, Msg * msg)
{
	uint32_t size = stringMap.size();
	rw_uint32_t( size, msg);

	if( msg->isWriting())
		for( std::map<std::string,std::string>::const_iterator it = stringMap.begin(); it != stringMap.end(); it++)
		{
			w_String((*it).first,  msg);
			w_String((*it).second, msg);
		}
	else
		for( unsigned i = 0; i < size; i++)
		{
			std::string name;
			rw_String( name, msg);
			std::string value;
			rw_String( value, msg);
			stringMap[name] = value;
		}
}

void af::rw_Int32_List( std::list<int32_t> &list, Msg * msg)
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

void af::rw_Int32_Vect( std::vector<int32_t> &vect, Msg * msg)
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

