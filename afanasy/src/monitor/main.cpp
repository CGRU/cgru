#include "../libafanasy/environment.h"

#include "../libafqt/qenvironment.h"

#include "monitorwindow.h"

#include <QtGui/QApplication>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

#ifndef WINNT
//####################### interrupt signal handler ####################################
#include <signal.h>
void sig_pipe(int signum)
{
   AFERROR("afanasy:: SIGPIPE:")
}
void sig_int(int signum)
{
   qApp->quit();
}
//#####################################################################################
#endif

MonitorWindow * pMONITOR;

bool running = true;

int main(int argc, char *argv[])
{
   uint32_t env_flags = af::Environment::Normal;
#ifdef WINNT
   af::Environment ENV( env_flags | af::Environment::Verbose, argc, argv);   // Verbose environment initialization
#else
   af::Environment ENV( env_flags, argc, argv);  // Silent environment initialization
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
      AFERROR("main: Environment initialization failed.")
      exit(1);
   }
   afqt::init( ENV.getMonitorWaitForConnected(), ENV.getMonitorWaitForReadyRead(), ENV.getMonitorWaitForBytesWritten());
   afqt::QEnvironment QENV( "monitor");
   if( !QENV.isValid())
   {
      AFERROR("main: QEnvironment initialization failed.")
      exit(1);
   }

   QApplication app(argc, argv);

   MonitorWindow monitor;
   pMONITOR = &monitor;
   if( !monitor.isInitialized())
   {
      AFERROR("main: Monitor initialization failed.")
      exit(1);
   }
   monitor.show();
   QObject::connect( &monitor, SIGNAL( stop()), &app, SLOT( quit()));

   int status = app.exec();

   printf("main: QApplication::exec: returned status = %d\n", status);

   return status;
}
