#include <arpa/inet.h>
#include <fcntl.h>
#include <memory.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <resolv.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>

#include <Python.h>

#include "../include/afjob.h"
#include "../libafanasy/environment.h"
#include "../libafnetwork/communications.h"
#include "../libafsql/name_afsql.h"

#include "afcommon.h"
#include "core.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Core* core;
bool running;

//####################### signal handlers ####################################
void sig_alrm(int signum)
{
   printf("ALARM: PID = %u, Thread ID = %lu.\n", (unsigned)getpid(), (long unsigned)pthread_self());
}
void sig_pipe(int signum)
{
   printf("PIPE ERROR: PID = %u, Thread ID = %lu.\n", (unsigned)getpid(), (long unsigned)pthread_self());
}
void sig_int(int signum)
{
   running = false;
}
//############################### child theads functions ################################
//
//######################## core thread to process client after accept #################
void* ThreadCore_processClient(void* arg)
{
  T_processClient__args * threadArgs = (T_processClient__args*)arg;

  if( core == NULL) return NULL;
// Disassociate from parent -
// Guarantees that thread resources are deallocated upon return.
   pthread_detach( pthread_self());
//
// run core function
//   core->theadReadMsg->process((struct T_processClient__args *)arg);
   core->threadReadMsg->process( threadArgs);
//
// close clien socket descriptor
//   close(((struct T_processClient__args *)arg)->client_sd);
   close( threadArgs->sd);
//
// free process arguments data
//   free(arg);
   delete threadArgs;

   return NULL;
}
//########################### main core thread #############################
void* ThreadCore_threadRun(void* arg)
{
   AFINFA("Thread (id = %lu) run created.\n", (long unsigned)pthread_self());
   core->threadRun->run();
   return NULL;
}
//########################### queued writing files thread #############################
void* ThreadCommon_dispatchMessages(void* arg)
{
// Unblock alarm signal.
   sigset_t sigmask;
   sigemptyset( &sigmask);
   sigaddset( &sigmask, SIGALRM);
   if( pthread_sigmask( SIG_UNBLOCK, &sigmask, NULL) != 0) perror("pthread_sigmask:");
//
   AFINFA("Thread (id = %lu) to dispatch messages created.\n", (long unsigned)pthread_self());
   AFCommon::MsgDispatchQueueRun();
   return NULL;
}
//########################### queued writing files thread #############################
void* ThreadCommon_writeFiles(void* arg)
{
   AFINFA("Thread (id = %lu) to write files created.\n", (long unsigned)pthread_self());
   AFCommon::FileWriteQueueRun();
   return NULL;
}
//########################### queued commands executing thread #############################
void* ThreadCommon_cmdExecQueue(void* arg)
{
   AFINFA("Thread (id = %lu) to execute commands created.\n", (long unsigned)pthread_self());
   AFCommon::CmdExecQueueRun();
   return NULL;
}
//########################### queued update tasks in database thread #############################
void* ThreadCommon_dbTasksUpdate(void* arg)
{
   AFINFA("Thread (id = %lu) update jobs tasks in database created.\n", (long unsigned)pthread_self());
   AFCommon::DBUpTaskQueueRun();
   return NULL;
}
//########################### queued update database thread #############################
void* ThreadCommon_dbUpdate(void* arg)
{
   AFINFA("Thread (id = %lu) update database created.\n", (long unsigned)pthread_self());
   AFCommon::DBUpdateQueueRun();
   return NULL;
}
//###################### server thread to accept clients #################################
void* ThreadServer_accept(void* arg)
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
   switch( protocol)
   {
      case AF_INET:
         printf("Using IPv4 addresses family.\n");
         break;
      case AF_INET6:
         printf("Using IPv6 addresses family.\n");
         printf("IPv4 connections addresses will be mapped to IPv6.\n");
         break;
      default:
         AFERROR("No addresses founed.\n");
         return NULL;
   }

//
// initializing server socket address:
   struct sockaddr_in server_sockaddr_in4;
   bzero( &server_sockaddr_in4, sizeof(server_sockaddr_in4));
   server_sockaddr_in4.sin_port = htons( af::Environment::getServerPort());
   server_sockaddr_in4.sin_addr.s_addr = INADDR_ANY;

   struct sockaddr_in6 server_sockaddr_in6;
   bzero( &server_sockaddr_in6, sizeof(server_sockaddr_in6));
   server_sockaddr_in6.sin6_port = htons( af::Environment::getServerPort());
//   server_sockaddr_in6.sin6_addr = IN6ADDR_ANY_INIT; // This is default value, it is zeros

//
// socket
   int server_sd = socket( protocol, SOCK_STREAM, 0);
   if( server_sd == -1)
   {
      AFERRPE("socket");
      return NULL;
   }
//
// set socket options for reuseing address immediatly after bind
   int value = 1;
   if( setsockopt( server_sd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) != 0)
      AFERRPE("set socket SO_REUSEADDR option failed");
//
// bind
   value = -1;
   if( protocol == AF_INET  ) value = bind( server_sd, (struct sockaddr*)&server_sockaddr_in4, sizeof(server_sockaddr_in4));
   if( protocol == AF_INET6 ) value = bind( server_sd, (struct sockaddr*)&server_sockaddr_in6, sizeof(server_sockaddr_in6));
   if( value != 0)
   {
      AFERRPE("bind");
      return NULL;
   }
//
// listen
   if( listen( server_sd, 9) != 0)
   {
      AFERRPE("listen");
      return NULL;
   }
   printf( "Listening %d port...\n", af::Environment::getServerPort());
   pthread_t childCore_processClient;
//
//############ accepting client connections:
   while( running )
   {
      if( core == NULL) break;

      T_processClient__args * threadArgs = new T_processClient__args;
      socklen_t client_sockaddr_len = sizeof(threadArgs->ss);

      threadArgs->sd = accept( server_sd, (struct sockaddr*)&(threadArgs->ss), &client_sockaddr_len);
      if( threadArgs->sd < 0)
      {
         AFERRPE("accept");
         continue;
      }
      if ( pthread_create(&childCore_processClient, NULL, ThreadCore_processClient, threadArgs) != 0 )
         AFERRPE("Pthread Process creation error");
//      ThreadCore_processClient( &threadArgs);
   }
   close( server_sd);

   AFINFO("ThreadServer_accept: exiting.\n");
   return NULL;
}
//
//#######################################################################################
//
//######################################## main #########################################
int main(int argc, char *argv[])
{
   QCoreApplication app( argc, argv);

   running = true;
   Py_InitializeEx(0);
   af::Environment ENV( af::Environment::NoFlags, argc, argv);
   if( af::init( af::InitFarm) == false) return 1;
   afsql::init();
   AFCommon afcommon;

   // create directories if it is not exists
   if( AFCommon::createDirectory( ENV.getTempDirectory().toUtf8().data(),         true) == false) return 1;
   if( AFCommon::createDirectory( ENV.getTasksStdOutDir().toUtf8().data(),        true) == false) return 1;
   if( AFCommon::createDirectory( ENV.getTasksStdOutDirDeleted().toUtf8().data(), true) == false) return 1;
   if( AFCommon::createDirectory( ENV.getJobsLogsDir().toUtf8().data(),           true) == false) return 1;
   if( AFCommon::createDirectory( ENV.getUsersLogsDir().toUtf8().data(),          true) == false) return 1;
   if( AFCommon::createDirectory( ENV.getRendersLogsDir().toUtf8().data(),        true) == false) return 1;

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
//
// init Core
   core = NULL;
   core = new Core();                             // create core object
   if( core == NULL )                             // check memory allocation
   {
      AFERROR("afanasy::main: can't allocate memory for core.\n");
      return 1;
   }
   if(!core->getInit())                           // check initializing success
   {
      AFERROR("afanasy::main: core init failed.\n");
      delete core;
      return 1;
   }
//
// spawn child threads
   // child threads ids
   pthread_t childServer_serverAccept;
   pthread_t childCore_run;
   pthread_t childCommon_dispatchMessages;
   pthread_t childCommon_writeFiles;
   pthread_t childCommon_cmdExecQueue;
   pthread_t childCommon_dbTasksUpdate;
   pthread_t childCommon_dbUpdate;

   AFINFA("Thread (id = %lu) is main, creating other threads.\n", (long unsigned)pthread_self());

   if( pthread_create( &childCommon_dispatchMessages, NULL, &ThreadCommon_dispatchMessages, NULL) != 0)
   {
      AFERRPE("afanasy::main: Common thread dispatchMessages creation error");
      return 1;
   }
   if( pthread_create( &childCommon_writeFiles, NULL, &ThreadCommon_writeFiles, NULL) != 0)
   {
      AFERRPE("afanasy::main: Common thread writeFiles creation error");
      return 1;
   }
   if( pthread_create( &childCommon_cmdExecQueue, NULL, &ThreadCommon_cmdExecQueue, NULL) != 0)
   {
      AFERRPE("afanasy::main: Common thread commands executing creation error");
      return 1;
   }
   if( pthread_create( &childCommon_dbTasksUpdate, NULL, &ThreadCommon_dbTasksUpdate, NULL) != 0)
   {
      AFERRPE("afanasy::main: Common thread update tasks in database creation error");
      return 1;
   }
   if( pthread_create( &childCommon_dbUpdate, NULL, &ThreadCommon_dbUpdate, NULL) != 0)
   {
      AFERRPE("afanasy::main: Common thread update database creation error");
      return 1;
   }
   if( pthread_create( &childCore_run, NULL, &ThreadCore_threadRun, NULL) != 0)
   {
      AFERRPE("afanasy::main: Core thread run creation error");
      return 1;
   }
   if( pthread_create( &childServer_serverAccept, NULL, &ThreadServer_accept, NULL) != 0)
   {
      AFERRPE("afanasy::main: Server thread accept creation error");
      return 1;
   }
//
// waiting for childs (do nothing)

   while( running) sleep(1);
   AFINFO("afanasy::main: Waiting child threads.\n");
   pthread_kill( childServer_serverAccept, SIGINT);
   AFINFO("afanasy::main: Joining accept thread.\n");
   pthread_join( childServer_serverAccept, NULL);


   AFINFO("afanasy::main: Joining run thread.\n");
   pthread_join( childCore_run,            NULL);

   AFCommon::MsgDispatchQueueQuit();
   AFINFO("afanasy::main: Joining dispatch messages thread.\n");
   pthread_join( childCommon_dispatchMessages,    NULL);

   AFCommon::FileWriteQueueQuit();
   AFINFO("afanasy::main: Joining writing files thread.\n");
   pthread_join( childCommon_writeFiles,    NULL);

   AFCommon::CmdExecQueueQuit();
   AFINFO("afanasy::main: Joining commands executing thread.\n");
   pthread_join( childCommon_cmdExecQueue,    NULL);

   AFCommon::DBUpTaskQueueQuit();
   AFINFO("afanasy::main: Joining update tasks in database thread.\n");
   pthread_join( childCommon_dbTasksUpdate,    NULL);

   AFCommon::DBUpdateQueueQuit();
   AFINFO("afanasy::main: Joining update database thread.\n");
   pthread_join( childCommon_dbUpdate,    NULL);


   AFINFO("afanasy::main: Deleting core:\n");
   if( core != NULL) delete core;

   AFINFO("afanasy::main: Output messages statistics:\n");
   af::destroy();
   Py_Finalize();

   return 0;
}
