#include "msgqueue.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <address.h>
#include <environment.h>
#include <msg.h>

#include <communications.h>

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

int socketfd;
struct sockaddr_in client_addr;

MsgQueue::MsgQueue( const QString & QueueName):
   AfQueue( QueueName)
{
   bzero(&client_addr, sizeof(client_addr));
   client_addr.sin_family = AF_INET;
}

MsgQueue::~MsgQueue()
{
}

void MsgQueue::processItem( AfQueueItem* item) const
{
   MsgAf * msg = (MsgAf*)item;

   const af::Address *address = msg->getAddress();
   if( address != NULL) send( msg, address);

   const std::list<af::Address*> * addresses = msg->getAddresses();
   if( addresses->size() < 1 ) return;

   std::list<af::Address*>::const_iterator it = addresses->begin();
   std::list<af::Address*>::const_iterator it_end = addresses->end();
   while( it != it_end)
   {
      if( *it != NULL ) send( msg, *it);
      it++;
   }
}

void MsgQueue::send( const MsgAf * msg, const af::Address * address) const
{
   address->setAddress( client_addr);
   //
   if ( (socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
   {
      AFERRPE("MsgQueue::processItem: socket");
      address->stdOut();
      return;
   }

   //
   //TODO client tcp socket malfunction or unreachable adress can block this core thread here
AFINFO("MsgQueue::processItem: tying to connect to client.\n");
   // Use SIGALRM to unblock
   if( alarm(2) != 0 )
      AFERROR("MsgQueue::send: alarm was already set.\n");

   if ( connect( socketfd, (struct sockaddr*)&client_addr, sizeof(client_addr)) != 0 )
   {
      AFERRPE("MsgQueue::processItem: connect");
      address->stdOut(); printf("\n");
      close(socketfd);
      alarm(0);
      return;
   }
   alarm(0);
   //
   // set socket maximum time to wait for an output operation to complete
   timeval so_sndtimeo;
   so_sndtimeo.tv_sec = af::Environment::getServer_SO_SNDTIMEO_SEC();
   so_sndtimeo.tv_usec = 0;
   if( setsockopt( socketfd, SOL_SOCKET, SO_SNDTIMEO, &so_sndtimeo, sizeof(so_sndtimeo)) != 0)
   {
      AFERRPE("MsgQueue::processItem: set socket SO_SNDTIMEO option failed");
      address->stdOut(); printf("\n");
      close(socketfd);
      return;
   }
   //
   // send
AFINFO("MsgQueue::processItem: tying to send message to client.\n");
   if(!com::msgsend( socketfd, msg))
   {
      AFERROR("MsgQueue::processItem: can't send message to client.\n");
      address->stdOut(); printf("\n");
   }

   close(socketfd);
}
