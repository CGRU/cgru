#include <QtGui/QApplication>

#include <environment.h>

#include "monitorwindow.h"

//#define AFOUTPUT
////#undef AFOUTPUT
#include <macrooutput.h>

#ifndef WINNT
//####################### interrupt signal handler ####################################
#include <signal.h>
void sig_pipe(int signum)
{
   AFERROR("afanasy:: SIGPIPE:\n");
}
void sig_int(int signum)
{
   qApp->quit();
}
//#####################################################################################
#endif

MonitorWindow * pMONITOR;

int main(int argc, char *argv[])
{
#ifdef WINNT
   af::Environment ENV( af::Environment::SolveServerAddress | af::Environment::Verbose, argc, argv);   // Verbose environment initialization
#else
   af::Environment ENV( af::Environment::SolveServerAddress, argc, argv);  // Silent environment initialization
//
// interrupt signal catch:
   struct sigaction actint;
   bzero( &actint, sizeof(actint));
   actint.sa_handler = sig_int;
   sigaction( SIGINT,  &actint, NULL);
   sigaction( SIGTERM, &actint, NULL);
   sigaction( SIGSEGV, &actint, NULL);
// SIGPIPE signal catch:
   struct sigaction actpipe;
   bzero( &actpipe, sizeof(actpipe));
   actpipe.sa_handler = sig_pipe;
   sigaction( SIGPIPE, &actpipe, NULL);
#endif
   if( !ENV.isValid())
   {
      AFERROR("main: Environment initialization failed.\n");
      exit(1);
   }
   afqt::init( ENV.getMonitorWaitForConnected(), ENV.getMonitorWaitForReadyRead(), ENV.getMonitorWaitForBytesWritten());

   QApplication app(argc, argv);

   MonitorWindow monitor;
   pMONITOR = &monitor;
   if( !monitor.isInitialized())
   {
      AFERROR("main: ChatDialog initialization failed.\n");
      exit(1);
   }
   monitor.show();
   QObject::connect( &monitor, SIGNAL( stop()), &app, SLOT( quit()));

   int status = app.exec();

   printf("main: QApplication::exec: returned status = %d\n", status);

   return status;
}
