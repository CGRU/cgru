#include <QtCore/QCoreApplication>

#include <name_afqt.h>
#include <environment.h>
#include <render.h>
#include "qobject.h"

#include <Python.h>

//#define AFOUTPUT
//#undef AFOUTPUT
#include <macrooutput.h>

Object * RenderObject;

#ifndef WINNT
//####################### interrupt signal handler ####################################
#include <signal.h>
void sig_pipe(int signum)
{
   AFERROR("afanasy:: SIGPIPE:\n");
}
void sig_int(int signum)
{
   RenderObject->exitRender();
}
//#####################################################################################
#endif

int main(int argc, char *argv[])
{
#ifdef WINNT
   af::Environment ENV( af::Environment::SolveServerAddress | af::Environment::Verbose, argc, argv);      // Verbose environment initialization
   Py_Initialize();
   if( af::init( af::InitServices | af::Verbose)  == false) return 1;
#else
   af::Environment ENV( af::Environment::SolveServerAddress, argc, argv );     // Silent environment initialization
//   Py_Initialize();
   Py_InitializeEx(0);
   if( af::init( af::InitServices) == false) return 1;
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

   afqt::init( ENV.getRenderWaitForConnected(), ENV.getRenderWaitForReadyRead(), ENV.getRenderWaitForBytesWritten());

   uint32_t state = af::Render::SOnline;
   if( argc > 1)
   {
      if( strcmp( argv[1], "nimby") == 0)
      {
         printf("Initial state set to \"nimby\"\n");
         state = state | af::Render::Snimby;
      }
      else if( strcmp( argv[1], "NIMBY") == 0)
      {
         printf("Initial state set to \"NIMBY\"\n");
         state = state | af::Render::SNIMBY;
      }
   }
   uint8_t priority = ENV.getPriority();

   QCoreApplication app( argc, argv);
   Object object( state, priority);
   RenderObject = &object;
   QObject::connect( &object, SIGNAL( exitApplication()), &app, SLOT( quit()));

   int retval = app.exec();

   af::destroy();
   Py_Finalize();

   printf("main: QApplication::exec: returned status = %d\n", retval);

   return retval;
}
