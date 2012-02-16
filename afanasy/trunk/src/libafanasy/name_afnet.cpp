#include "name_af.h"

#ifndef WINNT
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#endif

#include "address.h"
#include "environment.h"
#include "msg.h"
#include "msgstat.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

af::MsgStat mgstat;

/// Read data from file descriptor. Return bytes than was written or -1 on any error and prints an error in \c stderr.
int readdata( int fd, char* data, int data_len, int buffer_maxlen)
{
    AFINFA("com::readdata:: trying to recieve %d bytes.\n", data_len);
    int bytes = 0;
    while( bytes < data_len )
    {
        int r = read( fd, data+bytes, buffer_maxlen-bytes);
        if( r < 0)
        {
            AFERRPE("readdata: read");
            return -1;
        }
        AFINFA("readdata: read %d bytes.\n", r);
        if( r == 0) return 0;
        bytes += r;
    }

    return bytes;
}

/// Write data to file descriptor. Return \c false on any arror and prints an error in \c stderr.
bool writedata( int fd, char* data, int len)
{
   int written_bytes = 0;
   while( written_bytes < len)
   {
      int w = write( fd, data+written_bytes, len);
      if( w < 0)
      {
         AFERRPE("com::writedata:");
         return false;
      }
      written_bytes += w;
   }
   return true;
}

af::Msg * msgsendtoaddress( const af::Msg * i_msg, const af::Address & i_address,
                            bool & io_ok, af::VerboseMode i_verbose)
{
    io_ok = true;

    if( i_address.isEmpty() )
    {
        AFERROR("af::msgsend: Address is empty.")
        io_ok = false;
        return NULL;
    }

    int socketfd;
    struct sockaddr_storage client_addr;

    if( false == i_address.setSocketAddress( &client_addr)) return false;

    if(( socketfd = socket( client_addr.ss_family, SOCK_STREAM, 0)) < 0 )
    {
        AFERRPE("af::msgsend: socket() call failed")
        io_ok = false;
        return NULL;
    }

    AFINFO("af::msgsend: tying to connect to client.")
/*
    // Use SIGALRM to unblock
#ifndef WINNT
    if( alarm(2) != 0 )
        AFERROR("af::msgsend: alarm was already set.\n");
#endif //WINNT
*/
    if( connect(socketfd, (struct sockaddr*)&client_addr, i_address.sizeofAddr()) != 0 )
    {
        if( i_verbose == af::VerboseOn )
        {
            AFERRPA("af::msgsend: connect failure for msgType '%s':\n%s: ",
                af::Msg::TNAMES[i_msg->type()], i_address.generateInfoString().c_str())
        }
        close(socketfd);
/*
#ifndef WINNT
        alarm(0);
#endif //WINNT
*/
        io_ok = false;
        return NULL;
    }
/*
#ifndef WINNT
    alarm(0);
#endif //WINNT
*/
    //
    // set socket maximum time to wait for an output operation to complete
#ifndef WINNT
    timeval so_sndtimeo;
    so_sndtimeo.tv_sec = af::Environment::getServer_SO_SNDTIMEO_SEC();
    so_sndtimeo.tv_usec = 0;
    if( setsockopt( socketfd, SOL_SOCKET, SO_SNDTIMEO, &so_sndtimeo, sizeof(so_sndtimeo)) != 0)
    {
        AFERRPE("af::msgsend: set socket SO_SNDTIMEO option failed")
        i_address.stdOut(); printf("\n");
    }
    int nodelay = 1;
    if( setsockopt( socketfd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) != 0)
    {
       AFERRPE("af::msgsend: set socket TCP_NODELAY option failed");
       i_address.stdOut(); printf("\n");
    }
#endif //WINNT
    //
    // send
    if( false == af::msgwrite( socketfd, i_msg))
    {
        AFERRAR("af::msgsend: can't send message to client: %s",
                i_address.generateInfoString().c_str())
        close(socketfd);
        io_ok = false;
        return NULL;
    }

    if( false == i_msg->isReceiving())
    {
        close(socketfd);
        return NULL;
    }

    //
    // read answer
    af::Msg * o_msg = new af::Msg();
    if( false == af::msgread( socketfd, o_msg))
    {
       AFERROR("MsgAf::request: reading message failed.\n");
       close( socketfd);
       delete o_msg;
       io_ok = false;
       return NULL;
    }

    close(socketfd);
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
   mgstat.stdOut( columns, sorting);
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
    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_ADDRCONFIG;
 //   hints.ai_family = AF_UNSPEC; // This is value is default
    hints.ai_socktype = SOCK_STREAM;
    char service_port[16];
    sprintf( service_port, "%u", i_port);
    int err = getaddrinfo( i_name.c_str(), service_port, &hints, &res);
    if( err != 0 )
    {
        AFERRAR("com::connecttomaster:\n%s", gai_strerror(err))
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
            addr.stdOut();
        }

        // Free memory allocated for addresses:
        freeaddrinfo( res);

        return addr;

    }

    // Free memory allocated for addresses:
    freeaddrinfo( res);

    return af::Address();
}
/*
int af::connecttomaster( const std::string & i_name, int i_port, int i_type, VerboseMode i_verbose)
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
   int socketfd = -1;

   struct addrinfo *res;
   struct addrinfo hints;
   bzero(&hints, sizeof(hints));
   hints.ai_flags = AI_ADDRCONFIG;
//   hints.ai_family = AF_UNSPEC; // This is value is default
   hints.ai_socktype = SOCK_STREAM;
   char service_port[16];
   sprintf( service_port, "%u", i_port);
   int e = getaddrinfo( i_name.c_str(), service_port, &hints, &res);
   if( e != 0)
   {
      AFERRAR("com::connecttomaster:\n%s", gai_strerror(e));
      return socketfd;
   }
   for( struct addrinfo *r = res; r != NULL; r = r->ai_next)
   {
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
      // Skip address if type is forced
      if(( i_type != AF_UNSPEC ) && ( i_type != r->ai_family)) continue;

      socketfd = socket( r->ai_family, r->ai_socktype, r->ai_protocol);
      if( socketfd != -1 && connect( socketfd, r->ai_addr, r->ai_addrlen) == 0)
      break;
      if( socketfd != -1)
      {
         close( socketfd);
         socketfd = -1;
      }
   }

   // Free memory allocated for addresses:
   freeaddrinfo( res);

   return socketfd;
}
*/
bool af::msgread( int desc, af::Msg* msg)
{
AFINFO("com::msgread:\n");

   char * buffer = msg->buffer();
//
// Read message header data
   int bytes = ::readdata( desc, buffer, af::Msg::SizeHeader, af::Msg::SizeBuffer );
   if( bytes < af::Msg::SizeHeader)
   {
      AFERRAR("com::msgread: can't read message header, bytes = %d (< Msg::SizeHeader).\n", bytes);
      msg->setInvalid();
      return false;
   }
   if( msg->readHeader( bytes) == false)
   {
      AFERROR("com::msgread: constructing message header failed.\n");
      msg->setInvalid();
      return false;
   }
//
// Read message data if any
   if( msg->type() >= af::Msg::TDATA)
   {
      buffer = msg->buffer(); // buffer may be changed to fit new size
      int msgdatasize = msg->int32();
      int readlen = msgdatasize + af::Msg::SizeHeader - bytes;
      if( readlen > 0)
      {
         bytes = ::readdata( desc, buffer+bytes, readlen, readlen);
         if( bytes < readlen)
         {
            AFERRAR("com::msgread: read message data: ( bytes < readlen : %d < %d)\n", bytes, readlen);
            msg->setInvalid();
            return false;
         }
      }
   }

   mgstat.put( msg->type(), msg->writeSize());

   return true;
}

bool af::msgwrite( int i_desc, const af::Msg * i_msg)
{
    if( false == ::writedata( i_desc, i_msg->buffer(), i_msg->writeSize()))
    {
        AFERROR("com::msgsend: Error writing message.\n");
        return false;
    }

    mgstat.put( i_msg->type(), i_msg->writeSize());

    return true;
}

af::Msg * af::msgsend( Msg * i_msg, bool & io_ok, VerboseMode i_verbose )
{
    if( i_msg->isReceiving() && ( i_msg->addressesCount() > 0 ))
    {
        AFERROR("af::msgsend: Receiving message has several addresses.");
    }

    if( i_msg->addressIsEmpty() && ( i_msg->addressesCount() == 0 ))
    {
        AFERROR("af::msgsend: Message has no addresses to send to.");
        io_ok = false;
        i_msg->stdOut();
        return NULL;
    }

    if( false == i_msg->addressIsEmpty())
    {
        af::Msg * o_msg = ::msgsendtoaddress( i_msg, i_msg->getAddress(), io_ok, i_verbose);
        if( o_msg != NULL )
            return o_msg;
    }

    if( i_msg->addressesCount() < 1)
        return NULL;

    bool ok;
    const std::list<af::Address> * addresses = i_msg->getAddresses();
    std::list<af::Address>::const_iterator it = addresses->begin();
    std::list<af::Address>::const_iterator it_end = addresses->end();
    while( it != it_end)
    {
        ::msgsendtoaddress( i_msg, *it, ok, i_verbose);
        if( false == ok )
        {
            io_ok = false;
            // Store an address that message was failed to send to
            i_msg->setAddress( *it);
        }
        it++;
    }

    return NULL;
}
/*
bool af::msgRequest( const af::Msg * i_request, af::Msg * o_answer)
{
   if( i_request->addressIsEmpty() )
   {
      AFERROR("af::msgRequest: Request address is empty.")
      return false;
   }

   int socketfd;
   struct sockaddr_storage client_addr;

   i_request->getAddress().setSocketAddress( &client_addr);
   //
   // get socket descriptor
   if(( socketfd = socket( client_addr.ss_family, SOCK_STREAM, IPPROTO_TCP)) < 0 )
   {
      AFERRPE("MsgAf::request: socket");
      i_request->getAddress().stdOut(); printf("\n");
      return false;
   }
   //
   // connect
   if( connect( socketfd, (struct sockaddr*)&client_addr, i_request->getAddress().sizeofAddr()) != 0 )
   {
      AFERRPE("MsgAf::request: connect");
      i_request->getAddress().stdOut(); printf("\n");
      close(socketfd);
      return false;
   }
   //
   // set socket maximum time to wait for an output operation to complete
#ifndef WINNT
   timeval so_sndtimeo;
   so_sndtimeo.tv_sec = af::Environment::getServer_SO_SNDTIMEO_SEC();
   so_sndtimeo.tv_usec = 0;
   if( setsockopt( socketfd, SOL_SOCKET, SO_SNDTIMEO, &so_sndtimeo, sizeof(so_sndtimeo)) != 0)
   {
      AFERRPE("MsgAf::request: set socket SO_SNDTIMEO option failed");
   }
   int nodelay = 1;
   if( setsockopt( socketfd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) != 0)
   {
      AFERRPE("MsgAf::request: set socket TCP_NODELAY option failed");
   }
#endif// WINNT
   //
   // send request
   if( false == af::msgsend( socketfd, i_request))
   {
      AFERROR("MsgAf::request: can't send message to client.\n");
      i_request->stdOut();
      close( socketfd);
      return false;
   }
   //
   // read answer
   if( false == af::msgread( socketfd, o_answer))
   {
      AFERROR("MsgAf::request: reading message failed.\n");
      close( socketfd);
      return false;
   }

   close( socketfd);
   return true;
}
*/
/*
char * af::readdata( int fd, int & read_len)
{
   int buffer_len = af::Msg::SizeBuffer;  // Beginning buffer length
   char * buffer = new char[buffer_len];
   read_len = 0;
   for(;;)
   {
      int bytes = read( fd, buffer + read_len, buffer_len - read_len);
      AFINFA("af::readdata: recieved %d bytes.\n", bytes)
      if( bytes == 0 ) break;
      if( bytes == -1 ) // Error receiving data
      {
         read_len = -1; delete buffer; buffer = NULL;
         break;
      }
      read_len += bytes;
      if( read_len >= af::Msg::SizeBufferLimit) // Too much data recieved
      {
         read_len = -1; delete buffer; buffer = NULL;
         AFERRAR("Too big message received: len >= af::Msg::SizeBufferLimit : %d >= %d\n", read_len, af::Msg::SizeBufferLimit)
         break;
      }
      if( read_len == buffer_len)  // Need to increase buffer size
      {
         AFINFO("af::readdata: allocating new buffer.\n")
         char * old_buffer = buffer;
         int old_buffer_len = buffer_len;
         buffer_len = old_buffer_len << 1; // Increasing buffer twice
         buffer = new char[buffer_len];
         memcpy( buffer, old_buffer, read_len); // Copy recieved data into new buffer
         delete old_buffer;
      }
      else break;
   }
   AFINFA("af::readdata: returning %d bytes.\n", read_len)
   return buffer;
}
*/
