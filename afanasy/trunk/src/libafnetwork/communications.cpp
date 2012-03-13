#include "communications.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <netdb.h>

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mctaskup.h"

#include "msgstat.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

com::MsgStat mgstat;

void com::statwrite( af::Msg * msg)
{
   mgstat.writeStat( msg);
}
void com::statread( af::Msg * msg)
{
   mgstat.readStat( msg);
}

void com::statout( int columns, int sorting)
{
   mgstat.stdOut( columns, sorting);
}

bool com::writedata( int fd, char* data, int len)
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

int com::readdata( int fd, char* data, int len_min, int len_max)
{
   AFINFA("com::readdata:: trying to recieve %d bytes.\n", len_min);
   int bytes = 0;
   while( bytes < len_min )
   {
      int r = read( fd, data+bytes, len_max);
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

int com::connecttomaster( bool verbose, int type, const char * servername, int serverport)
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
      printf("failure %s\n", gai_strerror(e));
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

bool com::msgread( int desc, af::Msg* msg)
{
AFINFO("com::msgread:\n");

   char * buffer = msg->buffer();
//
// Read message header data
   int bytes = readdata( desc, buffer, af::Msg::SizeHeader, af::Msg::SizeBuffer );
   if( bytes < af::Msg::SizeHeader)
   {
      AFERRAR("com::msgread: can't read message header, bytes = %d (< Msg::SizeHeader).\n", bytes);
      return false;
   }
   if( msg->readHeader( bytes) == false)
   {
      AFERROR("com::msgread: constructing message header failed.\n");
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
         bytes = com::readdata( desc, buffer+bytes, readlen, readlen);
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
bool com::msgsend( int desc, const af::Msg *msg)
{
   if( !com::writedata( desc, msg->buffer(), msg->writeSize()))
   {
         AFERROR("com::msgsend: Error writing message.\n");
         return false;
   }

   mgstat.put( msg->type(),msg->writeSize());

   return true;
}
