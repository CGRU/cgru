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

#include "afcommon.h"
#include "jobcontainer.h"
#include "monitoraf.h"
#include "monitorcontainer.h"
#include "msgaf.h"
#include "msgqueue.h"
#include "rendercontainer.h"
#include "talkcontainer.h"
#include "threadargs.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

void processMessage( ThreadArgs * i_args);

MsgAf * readMessage( ThreadArgs * i_args);

MsgAf * threadProcessMsgCase( ThreadArgs * i_args, MsgAf * i_msg);

void writeMessage( ThreadArgs * i_args, MsgAf * i_msg);

// Accepted client thread entry point
void threadProcessMsg( void * i_args)
{
   ThreadArgs * threadArgs = (ThreadArgs*)i_args;

   // Message processing in separate function
   // to ensure that descriptor closed and
   // arguments are deleted in any way.
   processMessage( threadArgs);

   close(threadArgs->sd);

   delete threadArgs;
}

void processMessage( ThreadArgs * i_args)
{
   // Read message from socket
   MsgAf * msg_request = readMessage( i_args);

   if( msg_request == NULL )
   {
      // There was some error reading message
      return;
   }

#ifdef AFOUTPUT
printf("Request: ");msg_request->stdOut();
#endif

   // React on message, may be with response to the same opened socket
   MsgAf * msg_response = threadProcessMsgCase( i_args, msg_request);

   if( msg_response == NULL)
   {
      // No response needed, returning
      return;
   }

#ifdef AFOUTPUT
printf("Response: ");msg_response->stdOut();
#endif

   // Write response message back to client socket
   writeMessage( i_args, msg_response);

   delete msg_response;
}

MsgAf * readMessage( ThreadArgs * i_args)
{
   assert( i_args->sd > 0 );

   AFINFO( "ThreadReadMsg::msgProcess: trying to recieve message...")

   // set max allowed time to block recieveing data from client socket
   timeval so_rcvtimeo;
   so_rcvtimeo.tv_sec = af::Environment::getServer_SO_RCVTIMEO_SEC();
   so_rcvtimeo.tv_usec = 0;

   if( setsockopt( i_args->sd, SOL_SOCKET, SO_RCVTIMEO, &so_rcvtimeo, sizeof(so_rcvtimeo)) != 0)
   {
      perror( "setsockopt failed in ThreadReadMsg::process" );
      af::printAddress( &(i_args->ss));
      return NULL;
   }

   // reading message from client socket
   MsgAf * o_msg_request = new MsgAf( i_args->ss);
   if( false == com::msgread( i_args->sd, o_msg_request))
   {
      AFERROR("ThreadReadMsg::msgProcess: reading message failed.")
      af::printAddress( &(i_args->ss));
      delete o_msg_request;
      return NULL;
   }

   AFINFO( "ThreadReadMsg::msgProcess: message recieved.")

   return o_msg_request;
}

void writeMessage( ThreadArgs * i_args, MsgAf * i_msg)
{
   // set socket maximum time to wait for an output operation to complete
   timeval so_sndtimeo;
   so_sndtimeo.tv_sec = af::Environment::getServer_SO_SNDTIMEO_SEC();
   so_sndtimeo.tv_usec = 0;
   if( setsockopt( i_args->sd, SOL_SOCKET, SO_SNDTIMEO, &so_sndtimeo, sizeof(so_sndtimeo)) != 0)
   {
      AFERRPE("ThreadReadMsg::msgProcess: set socket SO_SNDTIMEO option failed")
      af::printAddress( &(i_args->ss));
      i_msg->stdOut();
      return;
   }

   // writing message back to client socket
   if( false == com::msgsend( i_args->sd, i_msg))
   {
      AFERROR("ThreadReadMsg::msgProcess: can't send message to client.")
      af::printAddress( &(i_args->ss));
      i_msg->stdOut();
      return;
   }

   AFINFO( "ThreadReadMsg::msgProcess: message sent.")
}
