#include "msgaf.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "afcommon.h"
#include "msgqueue.h"

#include <address.h>
#include <environment.h>

#include <communications.h>

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

MsgAf::MsgAf( int msgType, int msgInt):
   Msg( msgType, msgInt),
   address( NULL)
{
}

MsgAf::MsgAf( int msgType, Af * afClass ):
   Msg( msgType, afClass),
   address( NULL)
{
}

MsgAf::~MsgAf()
{
   if( address != NULL) delete address;
   while( addresses.size())
   {
      delete addresses.front();
      addresses.pop_front();
   }
}
/*
void MsgAf::setAddress( const af::Client* client)
{
   if( address != NULL) delete address;
   address = new af::Address( client->getAddress());
   if( address == NULL )
      AFERROR("MsgAf::setAddr( const Client* client): memory allocation failed.\n");
}
*/
void MsgAf::dispatch()
{
   AFCommon::QueueMsgDispatch( this);
   AFINFO("MsgAf::dispatch(): Message pushed into dispatch queue.\n");
}

bool MsgAf::request( MsgAf *answer)
{
   if( address == NULL )
   {
      AFERROR("MsgAf::dispatch: address == NULL\n");
      return false;
   }

   int socketfd;
   struct sockaddr_in client_addr;
   bzero(&client_addr, sizeof(client_addr));
   client_addr.sin_family = AF_INET;
   address->setAddress( client_addr);
   //
   // get socket descriptor
   if ( (socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
   {
      AFERRPE("MsgAf::request: socket");
      address->stdOut(); printf("\n");
      return false;
   }
   //
   // connect
   if ( connect( socketfd, (struct sockaddr*)&client_addr, sizeof(client_addr)) != 0 )
   {
      AFERRPE("MsgAf::request: connect");
      address->stdOut(); printf("\n");
      close(socketfd);
      return false;
   }
   //
   // set socket maximum time to wait for an output operation to complete
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
   //
   // send request
   if( !com::msgsend( socketfd, this))
   {
      AFERROR("MsgAf::request: can't send message to client.\n");
      stdOut();
      close( socketfd);
      return false;
   }
   //
   // read answer
   if( !com::msgread( socketfd, answer))
   {
      AFERROR("MsgAf::request: reading message failed.\n");
      close( socketfd);
      return false;
   }

   close( socketfd);
   return true;
}
