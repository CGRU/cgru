#include <QtGui/QApplication>
#include <QtGui/QIcon>

#include <environment.h>
#include <qenvironment.h>

#include "dialog.h"

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

int main(int argc, char *argv[])
{
#ifdef WINNT
   af::Environment ENV( af::Environment::SolveServerAddress | af::Environment::Verbose, argc, argv);   //< Verbose environment initialization
#else
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

//
// Silent environment initialization:
   af::Environment ENV( af::Environment::SolveServerAddress, argc, argv);
#endif
   if( !ENV.isValid())
   {
      AFERROR("main: Environment initialization failed.\n");
      exit(1);
   }

   afqt::init( ENV.getTalkWaitForConnected(), ENV.getTalkWaitForReadyRead(), ENV.getTalkWaitForBytesWritten());
   afqt::QEnvironment QENV( "watch");
   if( !QENV.isValid())
   {
      AFERROR("main: QEnvironment initialization failed.\n");
      exit(1);
   }

   QApplication app( argc, argv);
   app.setWindowIcon( QIcon( ENV.getAfRoot() + "/icons/talk_tray.png"));

   QPalette palette = app.palette();
   QENV.setPalette( palette);
   app.setPalette( palette);

   Dialog dialog;
   if( !dialog.initialized())
   {
      AFERROR("main: ChatDialog initialization failed.\n");
      exit(1);
   }
   dialog.show();
   QObject::connect( &dialog, SIGNAL( stop()), &app, SLOT( quit()));

   int status = app.exec();

   printf("main: QApplication::exec: returned status = %d\n", status);

   return status;
}
