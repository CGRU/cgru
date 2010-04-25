#include <QtGui/QApplication>
#include <QtGui/QIcon>

#define _USE_MATH_DEFINES
#include <math.h>

#include "../libafanasy/environment.h"

#include "../libafqt/qenvironment.h"

#include "dialog.h"
#include "item.h"
#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

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

   afqt::init( ENV.getWatchWaitForConnected(), ENV.getWatchWaitForReadyRead(), ENV.getWatchWaitForBytesWritten());
   afqt::QEnvironment QENV( "watch");
   if( !QENV.isValid())
   {
      AFERROR("main: QEnvironment initialization failed.\n");
      exit(1);
   }

   QApplication app(argc, argv);
   app.setWindowIcon( QIcon( ENV.getAfRoot() + "/icons/watch_dialog_22.png"));
   QPalette palette = app.palette();
   QENV.setPalette( palette);
   app.setPalette( palette);

   //
   // calculate star points:
   {
      float r = .4;
      float angle   = 90.0f/180*M_PI;
      float angle_d = 36.0f/180*M_PI;
      for( int i = 0; i < 10; i++)
      {
         Item::star_pointsInit[i].setX( cosf( angle));
         Item::star_pointsInit[i].setY(-sinf( angle));
         i++;
         angle += angle_d;
         Item::star_pointsInit[i].setX( cosf( angle)*r);
         Item::star_pointsInit[i].setY(-sinf( angle)*r);
         angle += angle_d;
      }
   }

   Dialog dialog;
   if( !dialog.isInitialized())
   {
      AFERROR("main: Dialog initialization failed.\n");
      exit(1);
   }

   Watch watch( &dialog, &app);

   dialog.show();
   QObject::connect( &dialog, SIGNAL( stop()), &app, SLOT( quit()));

   int status = app.exec();

   af::destroy();

   AFINFA("main: QApplication::exec: returned status = %d\n", status);

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
