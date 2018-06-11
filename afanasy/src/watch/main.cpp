#include "../libafanasy/environment.h"

#include "../libafqt/qenvironment.h"

#include "dialog.h"
#include "monitorhost.h"
#include "watch.h"

#include <QApplication>
#include <QtGui/QIcon>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

Dialog * ext_dialog = NULL;

#ifndef WINNT
//####################### interrupt signal handler ####################################
#include <signal.h>
void sig_pipe(int signum)
{
	AF_ERR << "SIGPIPE received.";
}
void sig_int(int signum)
{
	AF_WARN << "Interrupt signal received.";
	ext_dialog->close();
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
		AF_ERR << "Environment initialization failed.";
		exit(1);
   }

   afqt::QEnvironment QENV( "watch");
   if( !QENV.isValid())
   {
		AF_ERR << "QEnvironment initialization failed.";
		exit(1);
   }

   QApplication app(argc, argv);
   app.setWindowIcon( QIcon( afqt::stoq( ENV.getCGRULocation()) + "/icons/afwatch.png"));
	#if QT_VERSION >= 0x050000
	app.setStyle("fusion");
	#else
	app.setStyle("plastique");
	#endif

	MonitorHost monitor;

   Dialog dialog;
   if( !dialog.isInitialized())
   {
		AF_ERR << "Main dialog initialization failed.";
		exit(1);
   }
	ext_dialog = &dialog;

   Watch watch( &dialog, &app);

   dialog.show();
   QObject::connect( &dialog, SIGNAL( stop()), &app, SLOT( quit()));

   int status = app.exec();

	Watch::destroy();
   af::destroy();
   Py_Finalize();

	AF_DEBUG << "QApplication::exec: returned status = " << status;

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
