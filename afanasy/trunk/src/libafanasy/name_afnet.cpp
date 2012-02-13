#include "name_af.h"

#ifndef WINNT
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#else
#include <winsock2.h>
#endif

#include "address.h"
#include "environment.h"
#include "msg.h"
#include "msgstat.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

af::MsgStat mgstat;

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

bool af::writedata( int fd, char* data, int len)
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

int af::readdata( int fd, char* data, int data_len, int buffer_maxlen)
{
   AFINFA("com::readdata:: trying to recieve %d bytes.\n", data_len);
   int bytes = 0;
   while( bytes < data_len )
   {
//      int r = read( fd, data+bytes, buffer_maxlen);
      int r = read( fd, data+bytes, buffer_maxlen-bytes);
      if( r < 0)
      {
         AFERRPE("com::readdata:: read");
         return -1;
      }
      AFINFA("com::readdata:: read %d bytes.\n", r);
      if( r == 0) return 0;
      bytes += r;
   }

   return bytes;
}

int af::connecttomaster( bool verbose, int type, const char * servername, int serverport)
{
   if( verbose )
   {
      printf("Solving '%s'", servername);
      switch( type)
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
   sprintf( service_port, "%u", serverport);
   int e = getaddrinfo( servername, service_port, &hints, &res);
   if( e != 0)
   {
      AFERRAR("com::connecttomaster:\n%s", gai_strerror(e));
      return socketfd;
   }
   for( struct addrinfo *r = res; r != NULL; r = r->ai_next)
   {
      if( verbose)
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
      if(( type != AF_UNSPEC ) && ( type != r->ai_family)) continue;

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

char * af::readdata( int fd, int & read_len)
{
   int buffer_len = af::Msg::SizeBuffer;  // Beginning buffer length
   char * buffer = new char[buffer_len];
   read_len = 0;
   for(;;)
   {
      int bytes = read( fd, buffer + read_len, buffer_len - read_len);
      AFINFA("com::readdata: recieved %d bytes.\n", bytes)
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
         AFINFO("com::readdata: allocating new buffer.\n")
         char * old_buffer = buffer;
         int old_buffer_len = buffer_len;
         buffer_len = old_buffer_len << 1; // Increasing buffer twice
         buffer = new char[buffer_len];
         memcpy( buffer, old_buffer, read_len); // Copy recieved data into new buffer
         delete old_buffer;
      }
      else break;
   }
   AFINFA("com::readdata: returning %d bytes.\n", read_len)
   return buffer;
}

bool af::msgread( int desc, af::Msg* msg)
{
AFINFO("com::msgread:\n");

   char * buffer = msg->buffer();
//
// Read message header data
   int bytes = readdata( desc, buffer, af::Msg::SizeHeader, af::Msg::SizeBuffer );
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
         bytes = af::readdata( desc, buffer+bytes, readlen, readlen);
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
bool af::msgsend( int desc, const af::Msg *msg)
{
   if( false == af::writedata( desc, msg->buffer(), msg->writeSize()))
   {
         AFERROR("com::msgsend: Error writing message.\n");
         return false;
   }

   mgstat.put( msg->type(),msg->writeSize());

   return true;
}

bool af::msgRequest( const af::Msg * i_request, af::Msg * o_answer)
{
   if( i_request->addressIsEmpty() )
   {
      AFERROR("af::msgRequest: Request address is empty.")
      return false;
   }

   int socketfd;
   struct sockaddr_storage client_addr;

   i_request->getAddress().setSocketAddress( client_addr);
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
//   if ( connect( socketfd, (struct sockaddr*)&client_addr, sizeof(client_addr)) != 0 )
   if ( connect( socketfd, (struct sockaddr*)&client_addr, i_request->getAddress().sizeofAddr()) != 0 )
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
   if( false == msgsend( socketfd, i_request))
   {
      AFERROR("MsgAf::request: can't send message to client.\n");
      i_request->stdOut();
      close( socketfd);
      return false;
   }
   //
   // read answer
   if( false == msgread( socketfd, o_answer))
   {
      AFERROR("MsgAf::request: reading message failed.\n");
      close( socketfd);
      return false;
   }

   close( socketfd);
   return true;
}
