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

#if 0 // this was in main.cpp
void ThreadCommon_dispatchMessages(void* arg)
{
#ifndef _WIN32
   /* FIXME: why do we need this for? */
   // We need to unblok alarm ignal in dispatch messages thread.
   // SIGALRM was blocked in main.cpp, so all childs threads will ignore it.
   // Server uses this signal to unblock connect function - to limit connect time.
   // Client socket malfunction can block connect function - server should prevent it.
   sigset_t sigmask;
   sigemptyset( &sigmask);
   sigaddset( &sigmask, SIGALRM);
   if( pthread_sigmask( SIG_UNBLOCK, &sigmask, NULL) != 0)
      perror("pthread_sigmask:");
#endif

   AFINFA("Thread (id = %lu) to dispatch messages created.", (long unsigned)pthread_self())
   AFCommon::MsgDispatchQueueRun();
}
#endif
/*
  May be afqueue can have some virtual function,
  which will be called just after thread spawn,
  where thread can do some setup.
*/
MsgQueue::MsgQueue( const std::string & QueueName, bool i_start_thread ):
   AfQueue( QueueName, i_start_thread)
{
}

MsgQueue::~MsgQueue()
{
}

void MsgQueue::processItem( AfQueueItem* item)
{
   MsgAf * msg = (MsgAf*)item;

   if( false == msg->addressIsEmpty()) send( msg, msg->getAddress());

   const std::list<af::Address> * addresses = msg->getAddresses();
   if( addresses->size())
   {
      std::list<af::Address>::const_iterator it = addresses->begin();
      std::list<af::Address>::const_iterator it_end = addresses->end();
      while( it != it_end)
      {
         send( msg, *it);
         it++;
      }
   }
   delete msg;
}

void MsgQueue::send( const af::Msg * msg, const af::Address & address) const
{
   if( address.isEmpty() )
   {
      AFERROR("MsgQueue::send: Address is empty.")
      return;
   }

   int socketfd;
   struct sockaddr_storage client_addr;

   if( false == address.setSocketAddress( client_addr)) return;

   if(( socketfd = socket( client_addr.ss_family, SOCK_STREAM, 0)) < 0 )
   {
      perror("socket() call failed in MsgQueue::send");
      return;
   }

   AFINFO("MsgQueue::send: tying to connect to client.")

   // Use SIGALRM to unblock
   if( alarm(2) != 0 )
      AFERROR("MsgQueue::send: alarm was already set.\n");

   if( connect(socketfd, (struct sockaddr*)&client_addr, address.sizeofAddr()) != 0 )
   {
      AFERRPA("MsgQueue::send: connect failure for msgType '%s': %s",
         af::Msg::TNAMES[msg->type()], address.generateInfoString().c_str())
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
      AFERRPE("MsgQueue::send: set socket SO_SNDTIMEO option failed")
      address.stdOut(); printf("\n");
      close(socketfd);
      return;
   }
   //
   // send
   if( false == com::msgsend( socketfd, msg))
   {
      AFERRAR("MsgQueue::send: can't send message to client: %s", address.generateInfoString().c_str())
   }

   close(socketfd);
}
