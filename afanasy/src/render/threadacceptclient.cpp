#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

#ifdef WINNT
#include <winsock2.h>
#define sprintf sprintf_s
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#define closesocket close
#endif

#include "../libafanasy/msg.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/dlThread.h"

#include "renderhost.h"

#define AFOUTPUT
//#undef AFOUTPUT
#include "../include/macrooutput.h"

extern bool AFRunning;

void threadAcceptClient( void * i_arg )
{
    int protocol = AF_UNSPEC;

// Check for available local network addresses
    struct addrinfo hints, *res;
    memset( &hints, 0, sizeof(hints));
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

#if defined (WINNT)
    printf("Disable listening IPv6 for MS Windows.\n");
    protocol = AF_INET;
#elif defined (MACOSX)
    printf("Disable listening IPv6 for Mac OS X.\n");
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
    memset( &server_sockaddr_in4, 0, sizeof(server_sockaddr_in4));
    server_sockaddr_in4.sin_addr.s_addr = INADDR_ANY;
    server_sockaddr_in4.sin_family = AF_INET;

    struct sockaddr_in6 server_sockaddr_in6;
    memset( &server_sockaddr_in6, 0, sizeof(server_sockaddr_in6));
    server_sockaddr_in6.sin6_family = AF_INET6;
//   server_sockaddr_in6.sin6_addr = IN6ADDR_ANY_INIT; // This is default value, it is zeros

    int server_sd = socket( protocol, SOCK_STREAM, 0);
    if( server_sd == -1)
    {
        AFERRPE("socket")
        return;
    }

    int value = 1;
#ifdef WINNT
#define TOCHAR (char *)
#else
#define TOCHAR
    if( setsockopt( server_sd, SOL_SOCKET, SO_REUSEADDR, TOCHAR &value, sizeof(value)) != 0)
        AFERRPE("set socket SO_REUSEADDR option failed")
#endif

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
            RenderHost::setListeningPort( port);
            break;
        }
        #ifdef WINNT
        if ( value == SOCKET_ERROR )
            value = WSAGetLastError();
        if(( value == WSAEACCES        ) ||
           ( value == WSAEADDRINUSE    ) ||
           ( value == WSAEADDRNOTAVAIL ) ||
           ( value == WSAEINPROGRESS   ))
        #else
        if( errno == EADDRINUSE )
        #endif
        {
            port++;
            continue;
        }

        AFERRAR("bind() = %d\n", value);
        AFERRPE("bind()");
        AFRunning = false;
        return;
    }

    if( listen( server_sd, 9) != 0)
    {
        AFERRPE("listen()" );
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
                AFRunning = false;
                break;
            }
            if( false == AFRunning )
            {
                break;
            }

            af::sleep_sec( 1);

            continue;
        }

        af::Msg * msg_request = new af::Msg();

        // Read message data from socket
        if( false == af::msgread( sd, msg_request))
        {
            AFERROR("threadAcceptClient: reading message failed.")
            af::printAddress( &(ss));
            delete msg_request;
            closesocket(sd);
            continue;
        }

        switch( msg_request->type())
        {
        case af::Msg::TTaskOutputRequest:
        {
            af::MCTaskPos taskpos( msg_request);
            af::Msg * msg_response = new af::Msg();

            // Get task output immediately in this thread
            RenderHost::lockMutex();
            RenderHost::getTaskOutput( taskpos, msg_response);
            RenderHost::unLockMutex();

            // Write answer to the same socket
            if( false == af::msgwrite( sd, msg_response))
            {
                AFERROR("writeMessage: can't send message to client.")
                af::printAddress( &(ss));
                msg_response->v_stdOut();
            }
            delete msg_response;
            break;
        }
        default:
            RenderHost::acceptMessage( msg_request);
            closesocket(sd);
            continue;
        }

        closesocket(sd);
        delete msg_request;
    }

    closesocket( server_sd);

    AFINFO("threadAcceptClient: Finished.")
}
