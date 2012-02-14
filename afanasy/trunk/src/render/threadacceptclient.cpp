#include <arpa/inet.h>
#include <fcntl.h>
#include <memory.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>

#include "../libafanasy/msg.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/dlThread.h"

#include "renderhost.h"

#define AFOUTPUT
//#undef AFOUTPUT
#include "../include/macrooutput.h"

extern bool AFRunning;

//void threadProcessMsg( void * i_args);

void threadAcceptClient( void * i_arg )
{
    int protocol = AF_UNSPEC;

// Check for available local network addresses
    struct addrinfo hints, *res;
    bzero( &hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    char port_str[16];
    sprintf( port_str, "%u", af::Environment::getServerPort());
    getaddrinfo( NULL, port_str, &hints, &res);

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
#ifdef MACOSX
// FIXME: Current MAX OS can't listen IPv6?
    protocol = AF_INET;
#endif
    if( af::Environment::hasArgument("-noIPv6"))
    {
        printf("IPv6 is disabled.\n");
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
        return;
    }

//
// initializing server socket address:
    struct sockaddr_in server_sockaddr_in4;
    bzero( &server_sockaddr_in4, sizeof(server_sockaddr_in4));
    server_sockaddr_in4.sin_addr.s_addr = INADDR_ANY;
    server_sockaddr_in4.sin_family = AF_INET;

    struct sockaddr_in6 server_sockaddr_in6;
    bzero( &server_sockaddr_in6, sizeof(server_sockaddr_in6));
    server_sockaddr_in6.sin6_family = AF_INET6;
//   server_sockaddr_in6.sin6_addr = IN6ADDR_ANY_INIT; // This is default value, it is zeros

    int server_sd = socket( protocol, SOCK_STREAM, 0);
    if( server_sd == -1)
    {
        AFERRPE("socket")
        return;
    }
//
// set socket options for reuseing address immediatly after bind
    int value = 1;
    if( setsockopt( server_sd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) != 0)
        AFERRPE("set socket SO_REUSEADDR option failed")

    int port = af::Environment::getClientPort();
    int maxports = 0xffff-port;
    for( int i = 0; i <= maxports; i++)
    {
        if( protocol == AF_INET  )
        {
            server_sockaddr_in4.sin_port = htons( port);
            value = bind( server_sd, (struct sockaddr*)&server_sockaddr_in4, sizeof(server_sockaddr_in4));
        }
        else
        {
            server_sockaddr_in6.sin6_port = htons( port);
            value = bind( server_sd, (struct sockaddr*)&server_sockaddr_in6, sizeof(server_sockaddr_in6));
        }
        if( value == 0 )
        {
            af::Environment::setClientPort( port);
            break;
        }
        if( errno == EADDRINUSE )
        {
            port++;
            continue;
        }

        AFERRPE("bind()");
        AFRunning = false;
        return;
    }

    if( listen( server_sd, 9) != 0)
    {
        perror("listen()" );
        AFRunning = false;
        return;
    }

    printf( "Listening %d port...\n", af::Environment::getClientPort());

//
//############ accepting connections:
    while( AFRunning )
    {
        struct sockaddr_storage ss;
        socklen_t client_sockaddr_len = sizeof(ss);

        int sd = accept( server_sd, (struct sockaddr*)&(ss), &client_sockaddr_len);

        /* This is a cancellation point so the DlThread::Cancel can do its work. */
        DlThread::Self()->TestCancel();

        if( sd < 0)
        {
            AFERRPE("accept")
            switch( errno )
            {
            case EMFILE: // Very bad, probably main reading thread is locked, most likely server mutexes bug
                AFERROR("The per-process limit of open file descriptors has been reached.")
                break;
            case ENFILE: // Very bad, probably main reading thread is locked, most likely server mutexes bug
                AFERROR("The system limit on the total number of open files has been reached.")
                break;
            case EINTR:
                printf("Server was interrupted.\n");
                AFRunning = false;
                break;
            }
            if( false == AFRunning )
            {
                break;
            }
            sleep( 1);
            continue;
        }

        af::Msg * msg_request = new af::Msg();

        // Read message data from socket
        if( false == af::msgread( sd, msg_request))
        {
            AFERROR("threadAcceptClient: reading message failed.")
            af::printAddress( &(ss));
            delete msg_request;
            close(sd);
            continue;
        }

        RenderHost::acceptMessage( msg_request);
        close(sd);
        continue;

        af::Msg * msg_response = new af::Msg();

        // Write answer to socket
        if( false == af::msgsend( sd, msg_response))
        {
            AFERROR("writeMessage: can't send message to client.")
            af::printAddress( &(ss));
            msg_response->stdOut();
        }

        close(sd);
        delete msg_request;
        delete msg_response;
    }

    close( server_sd);

    AFINFO("threadAcceptClient: Finished.")
}
