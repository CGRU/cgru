#include "threadprocessmsg.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../libafanasy/address.h"
#include "../libafanasy/environment.h"

#include "../libafnetwork/communications.h"

#include "msgaf.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

void T_processClient__args::printAddress() const
{
   static const int buffer_len = 256;
   char buffer[buffer_len];
   const char * addr_str = NULL;
   uint16_t port = 0;
   printf("Address = ");
   switch( ss.ss_family)
   {
   case AF_INET:
   {
      struct sockaddr_in * sa = (struct sockaddr_in*)(&ss);
      port = sa->sin_port;
      addr_str = inet_ntoa( sa->sin_addr );
      break;
   }
   case AF_INET6:
   {
      struct sockaddr_in6 * sa = (struct sockaddr_in6*)(&ss);
      port = sa->sin6_port;
      addr_str = inet_ntop( AF_INET6, &(sa->sin6_addr), buffer, buffer_len);
      break;
   }
   default:
      printf("Unknown protocol");
      return;
   }
   if( addr_str )
   {
      printf("%s", addr_str);
      printf(" Port = %d", ntohs(port));
   }
   printf(" (descriptor=%d)", sd);
   printf("\n");
}

ThreadReadMsg::ThreadReadMsg( const ThreadPointers *ptrs):
   ThreadAf( ptrs)
{
}

ThreadReadMsg::~ThreadReadMsg()
{
AFINFO("ThreadReadMsg::~ThreadReadMsg:")
}

void ThreadReadMsg::run() {}

bool ThreadReadMsg::process( const struct T_processClient__args* theadArgs)
{
   int client_sd = theadArgs->sd;
   AFINFO( "ThreadReadMsg::msgProcess: trying to recieve message...")
   MsgAf* msg_request = new MsgAf( theadArgs->ss);

//   msg_request->setAddress( new af::Address( &(theadArgs->ss)));

   //
   // set socket maximum time to wait for an input operation to complete
   //
   // set max allowed time to block recieveing data from clien socket
   so_rcvtimeo.tv_sec = af::Environment::getServer_SO_RCVTIMEO_SEC();
   so_rcvtimeo.tv_usec = 0;
   // set max allowed time to block sending data to clien socket
   so_sndtimeo.tv_sec = af::Environment::getServer_SO_SNDTIMEO_SEC();
   so_sndtimeo.tv_usec = 0;
   if( setsockopt( client_sd, SOL_SOCKET, SO_RCVTIMEO, &so_rcvtimeo, sizeof(so_rcvtimeo)) != 0)
   {
      perror( "setsockopt failed in ThreadReadMsg::process" );
      theadArgs->printAddress();
      return false;
   }
   //
   // reading message from client socket
   if( false == com::msgread( client_sd, msg_request))
   {
      AFERROR("ThreadReadMsg::msgProcess: reading message failed.")
      theadArgs->printAddress();
      return false;
   }
   //
   // proseccing message from client socket
   AFINFO( "ThreadReadMsg::msgProcess: message recieved.")
#ifdef AFOUTPUT
printf("Request: ");msg_request->stdOut();
#endif
   MsgAf *msg_response = msgCase( msg_request);
   if( msg_response == NULL) return true;
#ifdef AFOUTPUT
printf("Response: ");msg_response->stdOut();
#endif
   //
   // set socket maximum time to wait for an output operation to complete
   if( setsockopt( client_sd, SOL_SOCKET, SO_SNDTIMEO, &so_sndtimeo, sizeof(so_sndtimeo)) != 0)
   {
      AFERRPE("ThreadReadMsg::msgProcess: set socket SO_SNDTIMEO option failed")
      theadArgs->printAddress();
      msg_response->stdOut();
      delete msg_response;
      return false;
   }
   //
   // writing message back to client socket
   if(!com::msgsend( client_sd, msg_response))
   {
      AFERROR("ThreadReadMsg::msgProcess: can't send message to client.")
      theadArgs->printAddress();
      msg_response->stdOut();
      delete msg_response;
      return false;
   }
   AFINFO( "ThreadReadMsg::msgProcess: message sent.")
   delete msg_response;
   return true;
}
