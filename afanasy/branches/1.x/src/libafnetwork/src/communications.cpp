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

#include <environment.h>
#include <msgclasses/mctaskup.h>

#include "msgstat.h"

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

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

int com::connecttomaster()
{
   int socketfd;
   if ( (socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
   {
      AFERRPE("com_connecttomaster::socket");
      return socketfd;
   }
   struct sockaddr_in master_addr;
   bzero(&master_addr, sizeof(master_addr));
   master_addr.sin_family = AF_INET;
   master_addr.sin_port = htons( af::Environment::getServerPort());
   struct hostent *hostinfo;
   hostinfo = gethostbyname( af::Environment::getServerName().toUtf8().data());
   memcpy( &master_addr.sin_addr, hostinfo->h_addr, hostinfo->h_length);
//
// Connect to server
   AFINFO("com_connecttomaster connecting...\n");
   if ( connect(socketfd, (struct sockaddr*)&master_addr, sizeof(sockaddr_in)) != 0 )
      AFERRPE("com_connecttomaster:: connect");
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
