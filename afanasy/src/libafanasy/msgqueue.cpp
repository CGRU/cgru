#include "msgqueue.h"

#ifndef WINNT
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#else
#include <winsock2.h>
#define close _close
#endif

#include "address.h"
#include "environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

#if 0 // this was in server main.cpp
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
MsgQueue::MsgQueue( const std::string & QueueName, StartTread i_start_thread ):
    AfQueue( QueueName, i_start_thread),
    m_returnQueue( NULL),
    m_returnNotSended( false),
    m_verbose( VerboseOn)
{
}

MsgQueue::~MsgQueue()
{
}

void MsgQueue::processItem( AfQueueItem* item)
{
    Msg * msg = (Msg*)item;

    if( msg == NULL )
    {
        AFERRAR("MsgQueue::processItem: '%s': NULL Meassage.\n", name.c_str())
        return;
    }

    if( msg->addressIsEmpty() && ( msg->addressesCount() == 0 ))
    {
        AFERRAR("MsgQueue::processItem: '%s':\n   Message has no addresses to send to.", name.c_str());
        msg->v_stdOut();
        delete msg;
        return;
    }

    bool ok;
    af::Msg * answer = af::msgsend( msg, ok, m_verbose);

    if( answer != NULL )
    {
        // We got an answer:
        if( m_returnQueue != NULL )
        {
            m_returnQueue->pushMsg( answer );
        }
        else
        {
            AFERRAR("MsgQueue::processItem: '%s':\n   Got an answer, but has no return queue set.", name.c_str())
            printf("Reuest: ");
            msg->v_stdOut();
            printf("Answer: ");
            answer->v_stdOut();
            delete answer;
        }
    }

    if(( false == ok) && m_returnNotSended )
    {
        if( m_returnQueue != NULL )
        {
            msg->setSendFailed();
            m_returnQueue->pushMsg( msg );
            return;
        }
        else
        {
            AFERRAR("MsgQueue::processItem: '%s':\n   Can't return message as no return queue set.", name.c_str())
            msg->v_stdOut();
        }
    }

    delete msg;
}
