#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef WINNT
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

#include "../include/afanasy.h"

#include "../libafanasy/common/dlThread.h"

#include "../libafanasy/environment.h"

#include "threadargs.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

extern bool AFRunning;

void threadProcessMsg( void * i_args);

void threadAcceptPort( void * i_arg, int i_port)
{
	AFINFA("Accept (id = %lu): %d - %d\n", (long unsigned)DlThread::Self(), i_port)

	ThreadArgs * threadArgs = (ThreadArgs*)i_arg;
	int protocol = AF_UNSPEC;

	// Check for available local network addresses
	struct addrinfo hints, *res;
	memset( &hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
	char port[16];
	sprintf( port, "%u", i_port);
	getaddrinfo( NULL, port, &hints, &res);

	printf("Available addresses:\n");

	for( struct addrinfo * ai = res; ai != NULL; ai = ai->ai_next)
	{
		switch( ai->ai_family)
		{
			case AF_INET:
			{
				if( protocol == AF_UNSPEC ) protocol = AF_INET;
				const char * addr_str = inet_ntoa( ((sockaddr_in*)(ai->ai_addr))->sin_addr );
				printf("IP = '%s'\n", addr_str);
				break;
			}
			case AF_INET6:
			{
				if(( protocol == AF_UNSPEC ) || ( protocol == AF_INET )) protocol = AF_INET6;
				static const int buffer_len = 256;
				char buffer[buffer_len];
				const char * addr_str = inet_ntop( AF_INET6, &(((sockaddr_in6*)(ai->ai_addr))->sin6_addr), buffer, buffer_len);
				printf("IPv6 = '%s'\n", addr_str);
				break;
			}
			default:
				printf("Unsupported address family, skipping.\n");
				continue;
		}
	}
	freeaddrinfo( res);

	if( af::Environment::isIPv6Disabled())
	{
		printf("IPv6 is disabled by config.\n");
		protocol = AF_INET;
	}

	switch(protocol)
	{
		case AF_INET:
			printf("Using IPv4 addresses family.\n");
			break;
		case AF_INET6:
			printf("Using IPv6 addresses family.\n");
			printf("IPv4 connections addresses will be mapped to IPv6.\n");
			break;
		default:
			AFERROR("No addresses founed.")
			AFRunning = false;
			return;
	}

	//
	// initializing server socket address:
	struct sockaddr_in server_sockaddr_in4;
	memset( &server_sockaddr_in4, 0, sizeof(server_sockaddr_in4));
	server_sockaddr_in4.sin_port = htons( af::Environment::getServerPort());
	server_sockaddr_in4.sin_addr.s_addr = INADDR_ANY;
	server_sockaddr_in4.sin_family = AF_INET;

	struct sockaddr_in6 server_sockaddr_in6;
	memset( &server_sockaddr_in6, 0, sizeof(server_sockaddr_in6));
	server_sockaddr_in6.sin6_port = htons( af::Environment::getServerPort());
	server_sockaddr_in6.sin6_family = AF_INET6;
	//   server_sockaddr_in6.sin6_addr = IN6ADDR_ANY_INIT; // This is default value, it is zeros

	int server_sd = socket( protocol, SOCK_STREAM, 0);
	if( server_sd == -1)
	{
		AFERRPE("socket")
		AFRunning = false;
		return;
	}
	//
	// set socket options for reuseing address immediatly after bind
	int value = 1;
	if( setsockopt( server_sd, SOL_SOCKET, SO_REUSEADDR, WINNT_TOCHAR(&value), sizeof(value)) != 0)
		AFERRPE("set socket SO_REUSEADDR option failed")

	value = -1;
	if( protocol == AF_INET  )
	{
		value = bind( server_sd, (struct sockaddr*)&server_sockaddr_in4, sizeof(server_sockaddr_in4));
	}
	else
	{
		#ifdef MACOSX
		assert( false );
		#endif
		assert( protocol == AF_INET6 );
		value = bind( server_sd, (struct sockaddr*)&server_sockaddr_in6, sizeof(server_sockaddr_in6));
	}

	if( value != 0)
	{
		AFERRAR("Port %d:", i_port)
		AFERRPE("bind()")
		AFRunning = false;
		return;
	}

	if( listen( server_sd, SOMAXCONN) != 0)
	{
		AFERRAR("Port %d:", i_port)
		AFERRPE("listen()")
		AFRunning = false;
		return;
	}

	printf( "Listening %d port...\n", af::Environment::getServerPort());

	DlThread process_one_client;

	//
	//############ accepting client connections:

	int error_wait; // Timeout to pause accepting on error
	static const int error_wait_max = 1 << 30;   // Maximum timeout value
	static const int error_wait_min = 1 << 3;    // Minimum timeout value
	error_wait = error_wait_min;

	int64_t accepts_count = 0;
	time_t  accepts_stat_count = 100;
	time_t  accepts_stat_time = time( NULL);

	while( AFRunning )
	{
		ThreadArgs * newThreadArgs = new ThreadArgs(*threadArgs);
		socklen_t client_sockaddr_len = sizeof(newThreadArgs->ss);

		newThreadArgs->sd = accept( server_sd, (struct sockaddr*)&(newThreadArgs->ss), &client_sockaddr_len);

		/* This is a cancellation point so the DlThread::Cancel can do its work. */
		DlThread::Self()->TestCancel();

		if( newThreadArgs->sd < 0)
		{
			AFERRPE("accept")
			switch( errno )
			{
				case EMFILE:
					AFERRAR("The per-process limit of open file descriptors %d has been reached.",
							af::Environment::getRLimit_NOFILE())
					break;
				case ENFILE:
					AFERROR("The system limit on the total number of open files has been reached.")
					break;
				case EINTR:
					printf("Server was interrupted.\n");
					AFRunning = false;
					break;
			}

			if( false == AFRunning )
			{
				delete threadArgs;
				break;
			}

			af::sleep_sec( error_wait);
			if( error_wait < error_wait_max)
				error_wait = error_wait << 1;

			continue;
		}

		error_wait = error_wait_min;

		/* Start a detached thread for this connection, the "t" object will be deleted
		automagically (check dlThread.cpp for details) and there is no need to join
		join this thread. */
		DlThread *t = new DlThread();
		t->SetDetached();

		/* Reduce new thread size */
		t->SetStackSize( 32768);

		/* The 'process' function will decode the incoming request and dispatch
		it to the proper queue. */
		int retval = t->Start( threadProcessMsg, newThreadArgs );

		if( retval != 0 )
		{
			// It seems that we can't raise a thread.
			std::string errstr;
			switch( retval)
			{
				case EAGAIN:
					errstr = "Insufficient resources to create another thread.";
					break;
				case EINVAL:
					errstr = "Invalid thread settings.";
					break;
				case EPERM:
					errstr = "No permission to set the scheduling policy and parameters specified.";
					break;
				default:
					errstr = "Unknown error.";
			}
			AF_ERR << errstr;

			af::socketDisconnect( newThreadArgs->sd, 0);
			delete newThreadArgs;
		}


		//
		// Server load statistics:
		//
		accepts_count++;
		if( accepts_count >= accepts_stat_count )
		{
			time_t cur_time = time( NULL);
			int seconds = cur_time - accepts_stat_time;
			if( seconds > 0 )
			{
				int accepts_per_second = accepts_count / seconds;

				#ifndef WINNT
				printf("\033[1;36m");
				#endif
				printf("Served connections per second: %d", accepts_per_second);
				#ifndef WINNT
				printf("\033[0m");
				#endif
				printf("\n");

				accepts_count = 0;
				accepts_stat_time = cur_time;
			}

			if( seconds < 10 )
				accepts_stat_count *= 10;
			else if(( seconds > 100 ) && ( accepts_stat_count > 100 ))
				accepts_stat_count /= 10;
		}
	}

	close( server_sd);

	printf("Thread Accept %d Finished.\n", i_port);
}

void threadAcceptClient( void * i_arg)
{
	threadAcceptPort( i_arg, af::Environment::getServerPort());
}

