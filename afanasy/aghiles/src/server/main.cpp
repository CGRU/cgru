#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>

#include "../libafanasy/dlThread.h"

#include <QtCore/QCoreApplication>

#include "../include/afjob.h"
#include "../libafanasy/environment.h"
#include "../libafnetwork/communications.h"
#include "../libafsql/name_afsql.h"

#include "afcommon.h"
#include "core.h"

#define AFOUTPUT
//#undef AFOUTPUT
#include "../include/macrooutput.h"

Core* core;
bool running;

//####################### signal handlers ####################################
void sig_alrm(int signum)
{
   printf("ALARM: PID = %u, Thread ID = %lu.\n", (unsigned)getpid(), (long unsigned)DlThread::Self());
}
void sig_pipe(int signum)
{
   printf("PIPE ERROR: PID = %u, Thread ID = %lu.\n", (unsigned)getpid(), (long unsigned)DlThread::Self());
}
void sig_int(int signum)
{
   fprintf( stderr, "SIG INT\n" );
   running = false;
}

//############################### child theads functions ################################
//
//######################## core thread to process client after accept #################
void ThreadCore_processClient(void* arg)
{
   T_processClient__args * threadArgs = (T_processClient__args*)arg;

   assert( threadArgs->sd > 0 );

   // Disassociate from parent -
   // Guarantees that thread resources are deallocated upon return.
   /* FIXME: why for ?? */
   //DlThread::Self()->Detach();

   assert( core );

   /* The 'process' method will decode the incoming request and dispatch
      it to the proper queue. */
   if( core != NULL)
      core->threadReadMsg->process( threadArgs );

   close(threadArgs->sd);
   delete threadArgs;
}

void ThreadCore_threadRun( void* arg )
{
   Core *core = (Core *)arg;
   core->threadRun->run();
}

#if 0
void ThreadCommon_dispatchMessages(void* arg)
{
#ifndef _WIN32
   /* FIXME: why do we need this for? */
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

//###################### server thread to accept clients #################################
void ThreadServer_accept(void* arg)
{
   int protocol = AF_UNSPEC;

// Check for available local network addresses
   struct addrinfo hints, *res;
   bzero( &hints, sizeof(hints));
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
   char port[16];
   sprintf( port, "%u", af::Environment::getServerPort());
   getaddrinfo( NULL, port, &hints, &res);

   printf("Available addresses:\n");

   for( struct addrinfo * ai = res; ai != NULL; ai = ai->ai_next)
   {
      switch( ai->ai_family)
      {
         case AF_INET:
         {
            if( protocol == AF_UNSPEC ) protocol = AF_INET;
            const char * addr_str = inet_ntoa( ((sockaddr_in*)(ai->ai_addr))->sin_addr );
            printf("IP = '%s'\n", addr_str);
            break;
         }
         case AF_INET6:
         {
            if(( protocol == AF_UNSPEC ) || ( protocol == AF_INET )) protocol = AF_INET6;
            static const int buffer_len = 256;
            char buffer[buffer_len];
            const char * addr_str = inet_ntop( AF_INET6, &(((sockaddr_in6*)(ai->ai_addr))->sin6_addr), buffer, buffer_len);
            printf("IPv6 = '%s'\n", addr_str);
            break;
         }
         default:
            printf("Unsupported address family, skipping.\n");
            continue;
      }
   }
   freeaddrinfo( res);
#ifdef MACOSX
// FIXME: Current MAX OS can't listen IPv6?
   protocol = AF_INET;
#endif

   switch(protocol)
   {
      case AF_INET:
         printf("Using IPv4 addresses family.\n");
         break;
      case AF_INET6:
         printf("Using IPv6 addresses family.\n");
         printf("IPv4 connections addresses will be mapped to IPv6.\n");
         break;
      default:
         AFERROR("No addresses founed.")
         return;
   }

//
// initializing server socket address:
   struct sockaddr_in server_sockaddr_in4;
   bzero( &server_sockaddr_in4, sizeof(server_sockaddr_in4));
   server_sockaddr_in4.sin_port = htons( af::Environment::getServerPort());
   server_sockaddr_in4.sin_addr.s_addr = INADDR_ANY;
   server_sockaddr_in4.sin_family = AF_INET;

   struct sockaddr_in6 server_sockaddr_in6;
   bzero( &server_sockaddr_in6, sizeof(server_sockaddr_in6));
   server_sockaddr_in6.sin6_port = htons( af::Environment::getServerPort());
   server_sockaddr_in6.sin6_family = AF_INET6;
//   server_sockaddr_in6.sin6_addr = IN6ADDR_ANY_INIT; // This is default value, it is zeros

   /* */
   int server_sd = socket( protocol, SOCK_STREAM, 0);
   if( server_sd == -1)
   {
      AFERRPE("socket")
      return;
   }
//
// set socket options for reuseing address immediatly after bind
   int value = 1;
   if( setsockopt( server_sd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) != 0)
      AFERRPE("set socket SO_REUSEADDR option failed");

   /* */
   value = -1;
   if( protocol == AF_INET  )
   {
      value = bind( server_sd, (struct sockaddr*)&server_sockaddr_in4, sizeof(server_sockaddr_in4));
   }
   else
   {
#ifdef MACOSX
      assert( false );
#endif
      assert( protocol == AF_INET6 );
      value = bind( server_sd, (struct sockaddr*)&server_sockaddr_in6, sizeof(server_sockaddr_in6));
   }

   if( value != 0)
   {
      perror( "bind() error in ThreadServer_accept" );
      return;
   }

   if( listen( server_sd, 9) != 0)
   {
      perror( "listen() error in ThreadServer_accept" );
      return; 
   }

   printf( "Listening %d port...\n", af::Environment::getServerPort());

   DlThread process_one_client;

//
//############ accepting client connections:
   int error_wait; // Timeout to pause accepting on error
   static const int error_wait_max = 1 << 30;   // Maximum timeout value
   static const int error_wait_min = 1 << 3;    // Minimum timeout value
   error_wait = error_wait_min;
   while( running )
   {
      assert( core );

      /* FIXME: can this happen? */
      if( core == NULL)
         break;

      T_processClient__args * threadArgs = new T_processClient__args;
      socklen_t client_sockaddr_len = sizeof(threadArgs->ss);

      threadArgs->sd = accept( server_sd, (struct sockaddr*)&(threadArgs->ss), &client_sockaddr_len);

      /* This is a cancellation point so the DlThread::Cancel can do
         its work. */
      DlThread::Self()->TestCancel();

      if( threadArgs->sd < 0)
      {
         AFERRPE("accept")
         switch( errno )
         {
            case EMFILE: // Very bad, probably main reading thread is locked, most likely server mutexes bug
               AFERROR("The per-process limit of open file descriptors has been reached.")
               break;
            case ENFILE: // Very bad, probably main reading thread is locked, most likely server mutexes bug
               AFERROR("The system limit on the total number of open files has been reached.")
               break;
            case EINTR:
               printf("Server was interrupted.\n");
               running = false;
               break;
         }
         if( false == running )
         {
            delete threadArgs;
            break;
         }
         sleep( error_wait);
         if( error_wait < error_wait_max) error_wait = error_wait << 1;
         continue;
      }
      error_wait = error_wait_min;

      /* Start a detached thread for this connection, the "t" object will be deleted
         automagically (check dlThread.cpp for details) and there is no need to join
         join this thread. */
      DlThread *t = new DlThread();
      t->SetDetached();
      t->Start( ThreadCore_processClient, threadArgs );
   }

   close( server_sd);

   AFINFO("ThreadServer_accept: exiting.")
}

//
//#######################################################################################
//
//######################################## main #########################################
int main(int argc, char *argv[])
{
   QCoreApplication app( argc, argv);

   running = true;
//   Py_InitializeEx(0);
   af::Environment ENV( af::Environment::NoFlags, argc, argv);
   if( af::init( af::InitFarm) == false) return 1;
   afsql::init();

   // create directories if it is not exists
   if( af::pathMakeDir( ENV.getTempDirectory(),  true) == false) return 1;
   if( af::pathMakeDir( ENV.getTasksStdOutDir(), true) == false) return 1;
   if( af::pathMakeDir( ENV.getUsersLogsDir(),   true) == false) return 1;
   if( af::pathMakeDir( ENV.getRendersLogsDir(), true) == false) return 1;

#ifndef _WIN32
   /* FIXME: do we _really_ need this ?  If someone kills the process 
      everytning will be fine anyway. */
//
// Interrupt signal catch:
   struct sigaction actint;
   bzero( &actint, sizeof(actint));
   actint.sa_handler = sig_int;
   sigaction( SIGINT,  &actint, NULL);
   sigaction( SIGTERM, &actint, NULL);
   sigaction( SIGSEGV, &actint, NULL);
// SIGPIPE signal catch
   struct sigaction actpipe;
   bzero( &actpipe, sizeof(actpipe));
   actpipe.sa_handler = sig_pipe;
   sigaction( SIGPIPE, &actpipe, NULL);

// SIGALRM signal catch and block
   struct sigaction actalrm;
   bzero( &actalrm, sizeof(actalrm));
   actalrm.sa_handler = sig_alrm;
   sigaction( SIGALRM, &actalrm, NULL);
   sigset_t sigmask;
   sigemptyset( &sigmask);
   sigaddset( &sigmask, SIGALRM);
   if( sigprocmask( SIG_BLOCK, &sigmask, NULL) != 0) perror("sigprocmask:");
   if( pthread_sigmask( SIG_BLOCK, &sigmask, NULL) != 0) perror("pthread_sigmask:");
#endif

   /*
      Create a core object.
      FIXME: explain in more details what is a core object.
   */
   core = new Core();
   assert( core->isInitialized() );

   if( !core->isInitialized() )
   {
      AFERROR("afanasy::main: core init failed.")
      delete core;
      return 1;
   }

   /*
      Creating the afcommon object will actually create many message queues
      that will spawn thres. Have a look in the implementation of AfCommon. 
   */ 
   AFCommon afcommon( core );

   AFINFA("Thread (id = %lu) is main, creating other threads.", (long unsigned)DlThread::Self())

   /*
      Start the thread that is responsible of listening to the port
      for incoming connections.
   */
   DlThread ServerAccept;
   ServerAccept.Start( &ThreadServer_accept, 0x0);

   /* NOTE: note sure why we call this the "core" thread. */
   DlThread CoreThread;
   CoreThread.Start( &ThreadCore_threadRun, core ); 
 
   /* Do nothing since everything is done in our threads. */
   while( running )
   {
      DlThread::Self()->Sleep( 1 );
   }

   AFINFO("afanasy::main: Waiting child threads.")
   //alarm(1);
   //ServerAccept.Cancel();
   //ServerAccept.Join();

   AFINFO("afanasy::main: Waiting Run.")
   CoreThread.Cancel();
   CoreThread.Join();

   delete core;

   AFINFO("afanasy::main: Output messages statistics:")
   af::destroy();

//   Py_Finalize();

   return 0;
}
