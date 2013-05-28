#include <memory.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "../libafanasy/environment.h"
#include "../libafanasy/dlThread.h"
#include "../libafanasy/msgqueue.h"

#include "../libafsql/dbconnection.h"

#include "afcommon.h"
#include "jobcontainer.h"
#include "monitorcontainer.h"
#include "sysjob.h"
#include "rendercontainer.h"
#include "talkcontainer.h"
#include "threadargs.h"
#include "usercontainer.h"


#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

extern bool AFRunning;

// Thread functions:
void threadAcceptClient( void * i_arg );
void threadAcceptClientHttp( void * i_arg );
void threadRunCycle( void * i_args);

//####################### signal handlers ####################################
void sig_int(int signum)
{
	fprintf( stderr, "SIG INT\n" );
	AFRunning = false;
}
void sig_alrm(int signum)
{
	printf("ALARM: Thread ID = %lu.\n", (long unsigned)DlThread::Self());
}
void sig_pipe(int signum)
{
	printf("PIPE ERROR: Thread ID = %lu.\n", (long unsigned)DlThread::Self());
}

//######################################## main #########################################
int main(int argc, char *argv[])
{
	// Initialize environment:
	af::Environment ENV( af::Environment::Server, argc, argv);
	ENV.addUsage("-noIPv6", "Disable IPv6.");

	// Initialize general library:
	if( af::init( af::InitFarm) == false) return 1;

	// Initialize database:
	afsql::init();

	// Environment aready printed usage and we can exit.
	if( ENV.isHelpMode()) return 0;

	// create directories if it is not exists
	if( af::pathMakePath( ENV.getTempDirectory(), af::VerboseOn ) == false) return 1;
	if( af::pathMakeDir( ENV.getTasksStdOutDir(), af::VerboseOn ) == false) return 1;
	if( af::pathMakeDir( ENV.getUsersLogsDir(),   af::VerboseOn ) == false) return 1;
	if( af::pathMakeDir( ENV.getRendersLogsDir(), af::VerboseOn ) == false) return 1;

// Server for windows can be me more simple and not use signals at all.
// Windows is not a server platform, so it designed for individual tests or very small companies with easy load.
#ifndef _WIN32
// Interrupt signals catch.
// We need to catch interrupt signals to let threads to finish running function themselves.
// This needed mostly fot queues to let them to finish to process last item.
	struct sigaction actint;
	bzero( &actint, sizeof(actint));
	actint.sa_handler = sig_int;
	sigaction( SIGINT,  &actint, NULL);
	sigaction( SIGTERM, &actint, NULL);

// SIGPIPE signal catch.
// This is not an error for our application.
	struct sigaction actpipe;
	bzero( &actpipe, sizeof(actpipe));
	actpipe.sa_handler = sig_pipe;
	sigaction( SIGPIPE, &actpipe, NULL);

// SIGALRM signal catch and block.
// Special threads use alarm signal to unblock connect function.
// Other threads should ignore this signal.
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

	// Create a separate database connection to register jobs.
	//  ( Job registration is can be long on heavy jobs.
	//    Server should use another connection for it not to stop
	//    database update on heavy job registration.
	//    WARINING!
	//    Server new jobs accept speed = database jobs fill-in speed!
	//    It is near 1000 tasks per second on common systems where
	//    PostgreSQL and Afanasy server are on the same host. )
	// Also use it now to restore all containers state from database.
	afsql::DBConnection afDB_JobRegister("AFDB_JobRegister");

	// containers initialization
	JobContainer jobs( &afDB_JobRegister);
	if( false == jobs.isInitialized()) return 1;
	JobAf::setJobContainer( &jobs);

	UserContainer users;
	if( false == users.isInitialized()) return 1;
	UserAf::setUserContainer( &users);

	RenderContainer renders;
	if( false == renders.isInitialized()) return 1;
	RenderAf::setRenderContainer( &renders);

	TalkContainer talks;
	if( false == talks.isInitialized()) return 1;

	MonitorContainer monitors;
	if( false == monitors.isInitialized()) return 1;
	
	MonitorAf::setMonitorContainer( &monitors);

	// Message Queue initialization, but without thread start.
	// Run cycle queue will read this messages itself.
	af::MsgQueue msgQueue("RunMsgQueue", af::AfQueue::e_no_thread);  
	if( false == msgQueue.isInitialized()) 
	  return 1;

	bool hasSystemJob = false;
//
// Open database to get nodes:
//
	afDB_JobRegister.DBOpen();
	if( afDB_JobRegister.isOpen())
	{
		// Update database tables:
		afsql::UpdateTables( &afDB_JobRegister);

		//
		// Get Renders from database:
		//
		printf("Getting renders from database...\n");
		std::list<int> rids = afDB_JobRegister.getIntegers( afsql::DBRender::dbGetIDsCmd());
		printf("%d renders founded.\n", (int)rids.size());
		for( std::list<int>::const_iterator it = rids.begin(); it != rids.end(); it++)
		{
			RenderAf * render = new RenderAf( *it);
			if( afDB_JobRegister.getItem( render))
				renders.addRender( render);
			else delete render;
		}
		printf("%d renders registered.\n", renders.getCount());

		//
		// Get Users from database:
		//
		printf("Getting users from database...\n");
		std::list<int> uids = afDB_JobRegister.getIntegers( afsql::DBUser::dbGetIDsCmd());
		printf("%d users founded.\n", (int)uids.size());
		for( std::list<int>::const_iterator it = uids.begin(); it != uids.end(); it++)
		{
			UserAf * user = new UserAf( *it);
			if( afDB_JobRegister.getItem( user)) users.addUser( user);
			else delete user;
		}
		printf("%d permanent users registered.\n", users.getCount());

		//
		// Get Jobs from database:
		//
		printf("Getting jobs from database...\n");
		std::list<int> jids = afDB_JobRegister.getIntegers( afsql::DBJob::dbGetIDsCmd());
		printf("%d jobs founded.\n", (int)jids.size());
		for( std::list<int>::const_iterator it = jids.begin(); it != jids.end(); it++)
		{
			JobAf * job = NULL;
			if( *it == AFJOB::SYSJOB_ID )
				job = new SysJob( SysJob::FromDataBase);
			else
				job = new JobAf( *it);
			if( afDB_JobRegister.getItem( job))
			{
				if( *it == AFJOB::SYSJOB_ID )
				{
					SysJob * sysjob = (SysJob*)job;
					if( sysjob->initSystem() )
					{
						printf("System job retrieved from database.\n");
						hasSystemJob = true;
					}
					else
					{
						printf("System job retrieved from database is obsolete. Deleting it...\n");
						std::list<std::string> queries;
						job->dbDeleteNoStatistics( &queries);
						delete job;
						afDB_JobRegister.execute( &queries);
						continue;
					}
				}
				jobs.job_register( job, &users, NULL);
			}
			else
			{
				printf("Deleting invalid job from database...\n");
				std::list<std::string> queries;
				job->dbDeleteNoStatistics( &queries);
				std::cout << queries.back() << std::endl;
				delete job;
				afDB_JobRegister.execute( &queries);
			}
		}
		printf("%d jobs registered.\n", jobs.getCount());

		//
		// Close database:
		//
		afDB_JobRegister.DBClose();
	}

//
// Create system maintenance job if it was not in database:
// (must be created after close of database connection to prevent mutex lock)
	if( hasSystemJob == false )
	{
		SysJob* job = new SysJob( SysJob::New);
		jobs.job_register( job, &users, NULL);
	}

	// Thread aruguments.
	ThreadArgs threadArgs;
	threadArgs.jobs      = &jobs;
	threadArgs.renders   = &renders;
	threadArgs.users     = &users;
	threadArgs.talks     = &talks;
	threadArgs.monitors  = &monitors;
	threadArgs.msgQueue  = &msgQueue;

	/*
	  Creating the afcommon object will actually create many message queues
	  that will spawn threads. Have a look in the implementation of AfCommon.
	*/
	AFCommon afcommon( &threadArgs );

	/*
	  Start the thread that is responsible of listening to the port
	  for incoming connections.
	*/
	DlThread ServerAccept;
	ServerAccept.Start( &threadAcceptClient, &threadArgs);

	//DlThread ServerAcceptHttp;
	//ServerAcceptHttp.Start( &threadAcceptClientHttp, &threadArgs);

	// Run cycle thread.
	// All 'brains' are there.
	DlThread RunCycleThread;
	RunCycleThread.Start( &threadRunCycle, &threadArgs);

	/* Do nothing since everything is done in our threads. */
	while( AFRunning )
	{
		DlThread::Self()->Sleep( 1 );
	}

	AFINFO("afanasy::main: Waiting child threads.")
	//alarm(1);
	/*FIXME: Why we don`t need to join accent thread? */
	//ServerAccept.Cancel();
	//ServerAccept.Join();

	AFINFO("afanasy::main: Waiting Run.")
	// No need to chanel run cycle thread as
	// every new cycle it checks running external valiable
	RunCycleThread.Join();

	af::destroy();

	return 0;
}
