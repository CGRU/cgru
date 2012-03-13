#include "theadprocessmsg.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <address.h>
#include <environment.h>
#include <communications.h>

#include "msgaf.h"

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

void T_processClient__args::printAddress() const
{
    printf("Address=%s:%d (sd=%d)\n", inet_ntoa(ip), ntohs(port), client_sd);
}

TheadReadMsg::TheadReadMsg( const TreadPointers *ptrs):
   TheadAf( ptrs)
{
}

TheadReadMsg::~TheadReadMsg()
{
AFINFO("TheadReadMsg::~TheadReadMsg:\n");
}

void TheadReadMsg::run() {}

bool TheadReadMsg::process( const struct T_processClient__args* theadArgs)
{
   int client_sd = theadArgs->client_sd;
   AFINFO( "TheadReadMsg::msgProcess: trying to recieve message...\n");
   MsgAf* msg_request = new MsgAf;
   af::Address* address = new af::Address( theadArgs->port, theadArgs->ip);
   msg_request->setAddress( address);

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
      AFERRPE("TheadReadMsg::msgProcess: set socket SO_RCVTIMEO option failed");
      theadArgs->printAddress();
      return false;
   }
   //
   // reading message from client socket
   if( false == com::msgread( client_sd, msg_request))
   {
      AFERROR("TheadReadMsg::msgProcess: reading message failed.\n");
      theadArgs->printAddress();
      return false;
   }
   //
   // proseccing message from client socket
   AFINFO( "TheadReadMsg::msgProcess: message recieved.\n");
#ifdef _DEBUG
   msg_request->stdOut();
#endif
   MsgAf *msg_response = msgCase( msg_request);
   if( msg_response == NULL) return true;
   //
   // set socket maximum time to wait for an output operation to complete
   if( setsockopt( client_sd, SOL_SOCKET, SO_SNDTIMEO, &so_sndtimeo, sizeof(so_sndtimeo)) != 0)
   {
      AFERRPE("TheadReadMsg::msgProcess: set socket SO_SNDTIMEO option failed");
      theadArgs->printAddress();
      msg_response->stdOut();
      delete msg_response;
      return false;
   }
   //
   // writing message back to client socket
   if(!com::msgsend( client_sd, msg_response))
   {
      AFERROR("TheadReadMsg::msgProcess: can't send message to client.\n");
      theadArgs->printAddress();
      msg_response->stdOut();
      delete msg_response;
      return false;
   }
   AFINFO( "TheadReadMsg::msgProcess: message sent.\n");
   delete msg_response;
   return true;
}
