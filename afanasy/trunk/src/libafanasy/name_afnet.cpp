#include "name_af.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "address.h"
#include "environment.h"
#include "msg.h"

#include "../libafnetwork/communications.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

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
   if( false == com::msgsend( socketfd, i_request))
   {
      AFERROR("MsgAf::request: can't send message to client.\n");
      i_request->stdOut();
      close( socketfd);
      return false;
   }
   //
   // read answer
   if( false == com::msgread( socketfd, o_answer))
   {
      AFERROR("MsgAf::request: reading message failed.\n");
      close( socketfd);
      return false;
   }

   close( socketfd);
   return true;
}
