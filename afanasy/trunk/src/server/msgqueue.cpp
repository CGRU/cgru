#include "msgqueue.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../libafanasy/address.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "../libafnetwork/communications.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"


MsgQueue::MsgQueue( const std::string & QueueName):
   AfQueue( QueueName)
{
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
//printf("MsgQueue::send:\n"); msg->stdOut();
   int socketfd;
   struct sockaddr_storage client_addr;

   address->setSocketAddress( &client_addr);

   if(( socketfd = socket( client_addr.ss_family, SOCK_STREAM, 0)) < 0 )
   {
      AFERRPE("MsgQueue::processItem: socket");
      address->stdOut(); printf("\n");
      return;
   }

   AFINFO("MsgQueue::processItem: tying to connect to client.\n");
   // Use SIGALRM to unblock
   if( alarm(2) != 0 )
      AFERROR("MsgQueue::send: alarm was already set.\n");

   if( connect( socketfd, (struct sockaddr*)&client_addr, sizeof(client_addr)) != 0 )
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
   if( false == com::msgsend( socketfd, msg))
   {
      AFERROR("MsgQueue::processItem: can't send message to client.\n");
      address->stdOut(); printf("\n");
   }

   close(socketfd);
}
