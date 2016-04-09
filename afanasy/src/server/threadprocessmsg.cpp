#include <stdio.h>
#include <stdlib.h>

#ifdef WINNT
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

#include "../include/afanasy.h"

#include "../libafanasy/address.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/msgqueue.h"

#include "afcommon.h"
#include "auth.h"
#include "jobcontainer.h"
#include "monitoraf.h"
#include "monitorcontainer.h"
#include "rendercontainer.h"
#include "threadargs.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

uint32_t processMessage( ThreadArgs * i_args);

bool readMessage( ThreadArgs * i_args, af::Msg * i_msg);

af::Msg * processHTTPGet( const af::Msg * i_msg);

af::Msg * threadProcessMsgCase( ThreadArgs * i_args, af::Msg * i_msg);

void writeMessage( ThreadArgs * i_args, af::Msg * i_msg);

// Accepted client thread entry point
void threadProcessMsg( void * i_args)
{
	ThreadArgs * threadArgs = (ThreadArgs*)i_args;

	// Message processing in separate function
	// to ensure that descriptor closed and
	// arguments are deleted in any way.
	uint32_t response_type = processMessage( threadArgs);

	af::socketDisconnect( threadArgs->sd, response_type);

	delete threadArgs;
}

uint32_t processMessage( ThreadArgs * i_args)
{
	uint32_t response_type = -1; // Error

	// Construct a new message to read.
	// Using a constructor that stores client address in message,
	// this client address will be used for a new client,
	// if it is a new client registration request.
	af::Msg * msg_request = new af::Msg( &(i_args->ss));
	af::Msg * msg_response = NULL;

	// Read message data from socket
	if( false == readMessage( i_args, msg_request))
	{
		// There was some error reading message
		delete msg_request;
		return response_type;
	}

#ifdef AFOUTPUT
printf("Request:  %s: %s\n", msg_request->v_generateInfoString().c_str(), msg_request->getAddress().v_generateInfoString().c_str());
#endif

	uint32_t request_type = msg_request->type();

	if( request_type == af::Msg::THTTPGET )
		msg_response = processHTTPGet( msg_request);

	// Check message IP trust mask:
	if(( msg_response == NULL ) && (false == msg_request->getAddress().matchIpMask()))
	{
		// Authenticate message that does not match trust mask:
		if( false == Auth::process( msg_request, &msg_response))
		{
			delete msg_request;
			return response_type;
		}
	}

	// React on message, may be with response to the same opened socket.
	if( msg_response == NULL )
		msg_response = threadProcessMsgCase( i_args, msg_request);
	// If request not needed any more it will be deleted there.

	response_type = 0; // No response

	if( msg_response == NULL)
	{
		// No response needed, returning
		return response_type;
	}

	// Set HTTP message type.
	// On writing header will be send first for web browsers.
	if( request_type == af::Msg::THTTP )
		msg_response->setTypeHTTP();
	else if(( request_type == af::Msg::TJSONBIN ) && ( response_type == af::Msg::TJSON ))
		msg_response->setJSONBIN();

#ifdef AFOUTPUT
printf("Response: "); msg_response->v_stdOut();
#endif

	response_type = msg_response->type();

	// Write response message back to client socket
	writeMessage( i_args, msg_response);

	delete msg_response;

	return response_type;
}

bool readMessage( ThreadArgs * i_args, af::Msg * io_msg)
{
	//AFINFO("Trying to recieve message...")

	// set max allowed time to block recieveing data from client socket
	if( af::Environment::getSO_Server_RCVTIMEO_sec() != -1 )
	{
		timeval so_rcvtimeo;
		so_rcvtimeo.tv_sec = af::Environment::getSO_Server_RCVTIMEO_sec();
		so_rcvtimeo.tv_usec = 0;
		if( setsockopt( i_args->sd, SOL_SOCKET, SO_RCVTIMEO, WINNT_TOCHAR(&so_rcvtimeo), sizeof(so_rcvtimeo)) != 0)
		{
			AFERRPE("Set socket SO_RCVTIMEO failed:");
			af::printAddress( &(i_args->ss));
		}
	}

	// set socket maximum time to wait for an output operation to complete
	if( af::Environment::getSO_Server_SNDTIMEO_sec() != -1 )
	{
		timeval so_sndtimeo;
		so_sndtimeo.tv_sec = af::Environment::getSO_Server_SNDTIMEO_sec();
		so_sndtimeo.tv_usec = 0;
		if( setsockopt( i_args->sd, SOL_SOCKET, SO_SNDTIMEO, WINNT_TOCHAR(&so_sndtimeo), sizeof(so_sndtimeo)) != 0)
		{
			AFERRPE("Set socket SO_SNDTIMEO option failed:")
			af::printAddress( &(i_args->ss));
		}
	}

	if( af::Environment::getSO_Client_TCP_NODELAY() != -1 )
	{
		int nodelay = af::Environment::getSO_Client_TCP_NODELAY();
		if( setsockopt( i_args->sd, IPPROTO_TCP, TCP_NODELAY, WINNT_TOCHAR(&nodelay), sizeof(nodelay)) != 0)
		{
			AFERRPE("Set socket TCP_NODELAY option failed:")
			af::printAddress( &(i_args->ss));
		}
	}

	if( af::Environment::getSO_Client_TCP_CORK() != -1 )
	{
		int nodelay = af::Environment::getSO_Client_TCP_CORK();
		if( setsockopt( i_args->sd, SOL_TCP, TCP_CORK, WINNT_TOCHAR(&nodelay), sizeof(nodelay)) != 0)
		{
			AFERRPE("Set socket TCP_CORK option failed:")
			af::printAddress( &(i_args->ss));
		}
	}


	// Reading message from client socket.
	if( false == af::msgread( i_args->sd, io_msg))
	{
		//AFERROR("readMessage: Reading message failed.")
		//af::printAddress( &(i_args->ss));
		return false;
	}

	//AFINFO("readMessage: Message recieved.")

	return true;
}

void writeMessage( ThreadArgs * i_args, af::Msg * i_msg)
{
	// writing message back to client socket
	if( false == af::msgwrite( i_args->sd, i_msg))
	{
		AFERROR("writeMessage: can't send message to client.")
		af::printAddress( &(i_args->ss));
		i_msg->stdOutData();
		return;
	}

	//AFINFO("writeMessage: message sent.")
}

bool validateGetFileName( const std::string & i_name);

af::Msg * processHTTPGet( const af::Msg * i_msg)
{
	//static const char header_OK[] = "HTTP/1.1 200 OK\r\n\r\n";
	//static const  int header_OK_len = strlen( header_OK);
	//static const char header_ERROR[] = "HTTP/1.1 404 Not Found\r\n\r\n";

	static const char tasks_file[] = "@TMP@";
	static const  int tasks_file_len = strlen( tasks_file);

	af::Msg * o_msg = new af::Msg();

	char * get = i_msg->data();
	int get_len = i_msg->dataLen();
	//::write( 1, get, get_len);
	int get_start = 4; // skipping "GET "
	int get_finish = get_start; 
	while( get[++get_finish] != ' ');
	while( get[get_start] == '/' ) get_start++;
	while( get[get_start] == '\\') get_start++;

	std::string file_name;
	if( get_finish - get_start > 1 )
	{
		file_name = std::string( get + get_start, get_finish - get_start);
		if( false == validateGetFileName( file_name))
		{
			AFCommon::QueueLogError("GET: Invalid file name from " + i_msg->getAddress().v_generateInfoString() + "\n" + file_name);
			file_name.clear();
		}
		else if( file_name.find( tasks_file) == 0 )
		{
			get_start += tasks_file_len;
			file_name = std::string( get + get_start, get_finish - get_start);
			if( file_name.find( af::Environment::getTempDir()) != 0 )
			{
				AFCommon::QueueLogError("GET: Invalid @TMP@ folder from " + i_msg->getAddress().v_generateInfoString() + "\n" + file_name);
				file_name.clear();
			}
//printf("GET TMP FILE: %s\n", file_name.c_str());
		}
		else
		{
			file_name = af::Environment::getHTTPServeDir() + AFGENERAL::PATH_SEPARATOR + file_name;
		}
	}
	else
	{
		file_name = af::Environment::getHTTPServeDir() + AFGENERAL::HTML_BROWSER;
	}

//printf("GET[%d,%d]=%s\n", get_start, get_finish, file_name.c_str());

	int file_size;
	char * file_data = NULL;
	if( file_name.size())
	{
		std::string error;
		file_data = af::fileRead( file_name, &file_size, -1, &error);
	}

	if( file_data )
	{
		char buffer[1024];
		sprintf( buffer, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", file_size);
		int buffer_len = strlen( buffer);

//		int msg_datalen = header_OK_len + file_size;
		int msg_datalen = buffer_len + file_size;
		char * msg_data = new char[msg_datalen];

//		memcpy( msg_data, header_OK, header_OK_len);
		memcpy( msg_data, buffer, buffer_len);
		memcpy( msg_data + buffer_len, file_data, file_size);

		o_msg->setData( msg_datalen, msg_data, af::Msg::THTTPGET);

		delete [] file_data;
		delete [] msg_data;
	}
	else
	{
		std::string error("HTTP/1.1 404 Not Found\r\n\r\n");
		error += "File not found: ";
		error += file_name;
		o_msg->setData( error.size(), error.c_str(), af::Msg::THTTPGET);
	}

	return o_msg;
}

static const char * files_skip[] = {
/*1*/"..",
/*2*/"htdigest",
/*3*/"htaccess",
/*4*/".json",
/*5*/":"
};
bool validateGetFileName( const std::string & i_name)
{
	for( int i = 0; i < 5; i++)
		if( i_name.find( files_skip[i]) != -1 ) return false;

	return true;
}

