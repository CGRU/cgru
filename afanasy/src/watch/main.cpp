#include "../libafanasy/environment.h"

#include "../libafqt/qenvironment.h"

#include "dialog.h"

#include "watch.h"

#include <QtGui/QApplication>
#include <QtGui/QIcon>

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

int main(int argc, char *argv[])
{
   Py_InitializeEx(0);
   uint32_t env_flags = af::Environment::AppendPythonPath;
#ifdef WINNT
   env_flags = env_flags | af::Environment::Verbose; // Verbose environment initialization
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
#endif
   af::Environment ENV( env_flags, argc, argv);  // Silent environment initialization

   if( !ENV.isValid())
   {
      AFERROR("main: Environment initialization failed.")
      exit(1);
   }

   afqt::init( ENV.getWatchWaitForConnected(), ENV.getWatchWaitForReadyRead(), ENV.getWatchWaitForBytesWritten());
   afqt::QEnvironment QENV( "watch");
   if( !QENV.isValid())
   {
      AFERROR("main: QEnvironment initialization failed.")
      exit(1);
   }

   QApplication app(argc, argv);
   app.setWindowIcon( QIcon( afqt::stoq( ENV.getCGRULocation()) + "/icons/afwatch.png"));

   Dialog dialog;
   if( !dialog.isInitialized())
   {
      AFERROR("main: Dialog initialization failed.")
      exit(1);
   }

   Watch watch( &dialog, &app);

   dialog.show();
   QObject::connect( &dialog, SIGNAL( stop()), &app, SLOT( quit()));

   int status = app.exec();

   af::destroy();
   Py_Finalize();

   AFINFA("main: QApplication::exec: returned status = %d", status)

   return status;
}
#ifdef WINNT
int WINAPI WinMain(
  __in  HINSTANCE hInstance,
  __in  HINSTANCE hPrevInstance,
  __in  LPSTR lpCmdLine,
  __in  int nCmdShow
)
{
   char * name = "afwatch";
   char ** ptr = &name;
   return main( 1, ptr);
}
#endif
