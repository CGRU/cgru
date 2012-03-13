#include <afjob.h>

#include <environment.h>

#include <communications.h>

#include <name_afsql.h>

#include "afcommon.h"
#include "core.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <memory.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <resolv.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>

#define AFOUTPUT
//#undef AFOUTPUT
#include <macrooutput.h>

Core* core;
int server_sd;
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
   if( core == NULL) return NULL;
// Disassociate from parent -
// Guarantees that thread resources are deallocated upon return.
   pthread_detach( pthread_self());
//
// run core function
   core->theadReadMsg->process((struct T_processClient__args *)arg);
//
// close clien socket descriptor
   close(((struct T_processClient__args *)arg)->client_sd);
//
// free process arguments data
   free(arg);
   return NULL;
}
//########################### main core thread #############################
void* ThreadCore_theadRun(void* arg)
{
   printf("Thread (id = %lu) run created.\n", (long unsigned)pthread_self());
   core->theadRun->run();
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
   printf("Thread (id = %lu) to dispatch messages created.\n", (long unsigned)pthread_self());
   AFCommon::MsgDispatchQueueRun();
   return NULL;
}
//########################### queued writing files thread #############################
void* ThreadCommon_writeFiles(void* arg)
{
   printf("Thread (id = %lu) to write files created.\n", (long unsigned)pthread_self());
   AFCommon::FileWriteQueueRun();
   return NULL;
}
//########################### queued commands executing thread #############################
void* ThreadCommon_cmdExecQueue(void* arg)
{
   printf("Thread (id = %lu) to execute commands created.\n", (long unsigned)pthread_self());
   AFCommon::CmdExecQueueRun();
   return NULL;
}
//########################### queued update tasks in database thread #############################
void* ThreadCommon_dbTasksUpdate(void* arg)
{
   printf("Thread (id = %lu) update jobs tasks in database created.\n", (long unsigned)pthread_self());
   AFCommon::DBUpTaskQueueRun();
   return NULL;
}
//########################### queued update database thread #############################
void* ThreadCommon_dbUpdate(void* arg)
{
   printf("Thread (id = %lu) update database created.\n", (long unsigned)pthread_self());
   AFCommon::DBUpdateQueueRun();
   return NULL;
}
//###################### server thread to accept clients #################################
void* ThreadServer_accept(void* arg)
{
//
// initializing server socket address:
   struct sockaddr_in server_sockaddr_in, client_sockaddr;
   bzero( &server_sockaddr_in, sizeof(server_sockaddr_in));
   server_sockaddr_in.sin_family = AF_INET;
   server_sockaddr_in.sin_addr.s_addr = INADDR_ANY;
   server_sockaddr_in.sin_port = htons( af::Environment::getServerPort());
   server_sd = socket( AF_INET, SOCK_STREAM, 0);
//
// socket
   if( server_sd == -1) { AFERRPE("afanasy:socket"); return NULL;}
   int value = 1;
//
// set socket options for reuseing address immediatly after bind
   if( setsockopt( server_sd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) != 0)
      AFERRPE("afanasy: set socket SO_REUSEADDR option failed");
//
// bind
   value = bind( server_sd, (struct sockaddr*)&server_sockaddr_in, sizeof(server_sockaddr_in));
   if( value != 0) { AFERRPE("afanasy:bind"); return NULL;}
//
// listen
   listen( server_sd, 3);
   if( value != 0) { AFERRPE("afanasy:listen"); return NULL;}
   printf( "Server listening...\n");
   int client_sd;
   int ans_len = sizeof(client_sockaddr);
   pthread_t childCore_processClient;
   struct T_processClient__args *threadArgs;
//
//############ accepting client connections:
   while( running )
   {
      if( core == NULL) break;
      client_sd = accept( server_sd, (struct sockaddr*)&client_sockaddr, (socklen_t*)&ans_len);
      if( client_sd < 0) { AFERRPE("afanasy: accept"); continue;}

      threadArgs = (struct T_processClient__args *) malloc(sizeof(struct T_processClient__args));
      threadArgs->client_sd = client_sd;
      threadArgs->ip = client_sockaddr.sin_addr;
      threadArgs->port = client_sockaddr.sin_port;

      if ( pthread_create(&childCore_processClient, NULL, ThreadCore_processClient, threadArgs) != 0 )
         AFERRPE("afanasy: Pthread Process creation error");
//      ThreadCore_processClient( &client_sd);
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
//
// initialize environment variables
//   QCoreApplication app( argc, argv);

   running = true;
   af::Environment ENV( af::Environment::NoFlags, argc, argv);
   if( af::init( af::InitFarm) == false) return 1;
   afsql::init();
   AFCommon afcommon;

   // create directories if it is not exists
   if( AFCommon::createDirectory( ENV.getTempDirectory().toUtf8().data(),  true) == false) return 1;
   if( AFCommon::createDirectory( ENV.getTasksStdOutDir().toUtf8().data(), true) == false) return 1;
   if( AFCommon::createDirectory( ENV.getJobsLogsDir().toUtf8().data(),    true) == false) return 1;
   if( AFCommon::createDirectory( ENV.getUsersLogsDir().toUtf8().data(),   true) == false) return 1;
   if( AFCommon::createDirectory( ENV.getRendersLogsDir().toUtf8().data(), true) == false) return 1;

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

   printf("Thread (id = %lu) is main, creating other threads.\n", (long unsigned)pthread_self());

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
   if( pthread_create( &childCore_run, NULL, &ThreadCore_theadRun, NULL) != 0)
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
   com::statwrite();
   com::statout( 1, true);
   af::destroy();

   return 0;
}
