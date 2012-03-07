#include "msgaf.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../libafanasy/address.h"
#include "../libafanasy/environment.h"

#include "../libafnetwork/communications.h"

#include "afcommon.h"
#include "msgqueue.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

MsgAf::MsgAf( int msgType, int msgInt):
      Msg( msgType, msgInt)
{
}

MsgAf::MsgAf( int msgType, Af * afClass ):
      Msg( msgType, afClass)
{
}

MsgAf::MsgAf( const struct sockaddr_storage & ss):
      address( ss)
{
}

MsgAf::~MsgAf()
{
   /*
   if( address != NULL) delete address;
   while( addresses.size())
   {
      delete addresses.front();
      addresses.pop_front();
   }
   */
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
   if( address.isEmpty() )
   {
      AFERROR("MsgAf::dispatch: Address is empty\n")
      return false;
   }

   int socketfd;
   struct sockaddr_storage client_addr;

   address.setSocketAddress( client_addr);
   //
   // get socket descriptor
   if(( socketfd = socket( client_addr.ss_family, SOCK_STREAM, IPPROTO_TCP)) < 0 )
   {
      AFERRPE("MsgAf::request: socket");
      address.stdOut(); printf("\n");
      return false;
   }
   //
   // connect
//   if ( connect( socketfd, (struct sockaddr*)&client_addr, sizeof(client_addr)) != 0 )
   if ( connect( socketfd, (struct sockaddr*)&client_addr, address.sizeofAddr()) != 0 )
   {
      AFERRPE("MsgAf::request: connect");
      address.stdOut(); printf("\n");
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
   if( false == com::msgsend( socketfd, this))
   {
      AFERROR("MsgAf::request: can't send message to client.\n");
      stdOut();
      close( socketfd);
      return false;
   }
   //
   // read answer
   if( false == com::msgread( socketfd, answer))
   {
      AFERROR("MsgAf::request: reading message failed.\n");
      close( socketfd);
      return false;
   }

   close( socketfd);
   return true;
}
