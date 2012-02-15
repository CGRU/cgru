#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../libafanasy/address.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/msgqueue.h"

#include "afcommon.h"
#include "jobcontainer.h"
#include "monitoraf.h"
#include "monitorcontainer.h"
#include "rendercontainer.h"
#include "talkcontainer.h"
#include "threadargs.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

void processMessage( ThreadArgs * i_args);

bool readMessage( ThreadArgs * i_args, af::Msg * i_msg);

af::Msg * threadProcessMsgCase( ThreadArgs * i_args, af::Msg * i_msg);

void writeMessage( ThreadArgs * i_args, af::Msg * i_msg);

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
    // Construct a new message to read.
    // Using a constructor that stores client address in message,
    // this client address will be used for a new client,
    // if it is a new client registration request.
    af::Msg * msg_request = new af::Msg( &(i_args->ss));

    // Check message IP mask:
    if( false == msg_request->getAddress().matchIpMask())
    {
        // IP address does not match mask, no other operation allowed
        AFCommon::QueueLogError( std::string("Not allowed incoming message IP address: "
                                             + msg_request->getAddress().generateInfoString()));
        delete msg_request;
        return;
    }

    // Read message data from socket
    if( false == readMessage( i_args, msg_request))
    {
        // There was some error reading message
        delete msg_request;
        return;
    }

#ifdef AFOUTPUT
printf("Request: ");msg_request->stdOut();
#endif

   // React on message, may be with response to the same opened socket.
   af::Msg * msg_response = threadProcessMsgCase( i_args, msg_request);
   // If request not needed any more it will be deleted there.

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

bool readMessage( ThreadArgs * i_args, af::Msg * io_msg)
{
   AFINFO("Trying to recieve message...")

   // set max allowed time to block recieveing data from client socket
   timeval so_rcvtimeo;
   so_rcvtimeo.tv_sec = af::Environment::getServer_SO_RCVTIMEO_SEC();
   so_rcvtimeo.tv_usec = 0;

   if( setsockopt( i_args->sd, SOL_SOCKET, SO_RCVTIMEO, &so_rcvtimeo, sizeof(so_rcvtimeo)) != 0)
   {
      AFERRPE("readMessage: setsockopt failed.");
      af::printAddress( &(i_args->ss));
      return false;
   }

   // Reading message from client socket.
   if( false == af::msgread( i_args->sd, io_msg))
   {
      AFERROR("readMessage: Reading message failed.")
      af::printAddress( &(i_args->ss));
      return false;
   }

   AFINFO("readMessage: Message recieved.")

   return true;
}

void writeMessage( ThreadArgs * i_args, af::Msg * i_msg)
{
   // set socket maximum time to wait for an output operation to complete
   timeval so_sndtimeo;
   so_sndtimeo.tv_sec = af::Environment::getServer_SO_SNDTIMEO_SEC();
   so_sndtimeo.tv_usec = 0;
   if( setsockopt( i_args->sd, SOL_SOCKET, SO_SNDTIMEO, &so_sndtimeo, sizeof(so_sndtimeo)) != 0)
   {
      AFERRPE("writeMessage: set socket SO_SNDTIMEO option failed")
      af::printAddress( &(i_args->ss));
      i_msg->stdOut();
      return;
   }

   // writing message back to client socket
   if( false == af::msgwrite( i_args->sd, i_msg))
   {
      AFERROR("writeMessage: can't send message to client.")
      af::printAddress( &(i_args->ss));
      i_msg->stdOut();
      return;
   }

   AFINFO("writeMessage: message sent.")
}
