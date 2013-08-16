#include <stdio.h>
#include <stdlib.h>

#ifdef WINNT
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#define closesocket close
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
#include "talkcontainer.h"
#include "threadargs.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

void processMessage( ThreadArgs * i_args);

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
	processMessage( threadArgs);

	closesocket(threadArgs->sd);

	delete threadArgs;
}

void processMessage( ThreadArgs * i_args)
{
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
		return;
	}

#ifdef AFOUTPUT
printf("Request:  %s: %s\n", msg_request->v_generateInfoString().c_str(), msg_request->getAddress().v_generateInfoString().c_str());
#endif

	if( msg_request->type() == af::Msg::THTTPGET )
		msg_response = processHTTPGet( msg_request);

	// Check message IP trust mask:
	if(( msg_response == NULL ) && (false == msg_request->getAddress().matchIpMask()))
	{
		// Authenticate message that does not match trust mask:
		if( false == Auth::process( msg_request, &msg_response))
		{
			delete msg_request;
			return;
		}
	}

	// React on message, may be with response to the same opened socket.
	if( msg_response == NULL )
		msg_response = threadProcessMsgCase( i_args, msg_request);
	// If request not needed any more it will be deleted there.

	if( msg_response == NULL)
	{
		// No response needed, returning
		return;
	}

	// Set HTTP message type.
	// On writing header will be send first for web browsers.
	if( msg_request->type() == af::Msg::THTTP )
		msg_response->setTypeHTTP();

#ifdef AFOUTPUT
printf("Response: "); msg_response->v_stdOut();
#endif

	// Write response message back to client socket
	writeMessage( i_args, msg_response);

	delete msg_response;
}

bool readMessage( ThreadArgs * i_args, af::Msg * io_msg)
{
	//AFINFO("Trying to recieve message...")

	// set max allowed time to block recieveing data from client socket
	timeval so_rcvtimeo;
	so_rcvtimeo.tv_sec = af::Environment::getServer_SO_RCVTIMEO_SEC();
	so_rcvtimeo.tv_usec = 0;

	#ifdef WINNT
	#define TOCHAR (char *)
	#else
	#define TOCHAR
	#endif

	if( setsockopt( i_args->sd, SOL_SOCKET, SO_RCVTIMEO, TOCHAR(&so_rcvtimeo), sizeof(so_rcvtimeo)) != 0)
	{
		AFERRPE("readMessage: setsockopt failed.");
		af::printAddress( &(i_args->ss));
		return false;
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
	// set socket maximum time to wait for an output operation to complete
	timeval so_sndtimeo;
	so_sndtimeo.tv_sec = af::Environment::getServer_SO_SNDTIMEO_SEC();
	so_sndtimeo.tv_usec = 0;

	#ifdef WINNT
	#define TOCHAR (char *)
	#else
	#define TOCHAR
	#endif

	if( setsockopt( i_args->sd, SOL_SOCKET, SO_SNDTIMEO, TOCHAR(&so_sndtimeo), sizeof(so_sndtimeo)) != 0)
	{
		AFERRPE("writeMessage: set socket SO_SNDTIMEO option failed")
		af::printAddress( &(i_args->ss));
		i_msg->v_stdOut();
		return;
	}

	// writing message back to client socket
	if( false == af::msgwrite( i_args->sd, i_msg))
	{
		AFERROR("writeMessage: can't send message to client.")
		af::printAddress( &(i_args->ss));
		i_msg->v_stdOut();
		return;
	}

	//AFINFO("writeMessage: message sent.")
}

af::Msg * processHTTPGet( const af::Msg * i_msg)
{
	char * get = i_msg->data();
	int get_len = i_msg->dataLen();
	//::write( 1, get, get_len);

	int get_start = 4; // skipping "GET "
	int get_finish = get_start; 
	char * data = NULL;
	int datalen;
	std::string datafile;
	while( get[++get_finish] != ' ');
	while( get[get_start] == '/' ) get_start++;
	while( get[get_start] == '\\') get_start++;
	if( get_finish - get_start > 1 )
	{
		datafile = std::string( get + get_start, get_finish - get_start);
printf("GET[%d,%d]=%s\n", get_start, get_finish, datafile.c_str());
		if(( datafile.find("..") == -1 ) && ( datafile.find(':') == -1 ))
		{
			datafile = af::Environment::getCGRULocation() + AFGENERAL::PATH_SEPARATOR + datafile;
			std::string error;
			data = af::fileRead( datafile, &datalen, -1, &error);
		}
	}
	else
	{
		datafile = af::Environment::getAfRoot() + AFGENERAL::HTML_BROWSER;
		data = af::fileRead( datafile, &datalen);
	}

	if( data == NULL )
	{
		static char httpError[] = "HTTP/1.0 404 Not Found\r\n\r\n";
		//writedata( i_desc, httpError, strlen(httpError));
		data = httpError;
		datalen = strlen(httpError);
	}
/*	else
	{
		static const char httpHeader[] = "HTTP/1.0 200 OK\r\n\r\n";
		writedata( i_desc, httpHeader, strlen(httpHeader));
		writedata( i_desc, data, datalen);
	}*/

	af::Msg * o_msg = new af::Msg();
	o_msg->setData( datalen, data, af::Msg::THTTPGET);

	return o_msg;
}
