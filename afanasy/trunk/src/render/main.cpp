#include <QtCore/QCoreApplication>

#include <Python.h>

#include "../libafanasy/environment.h"
#include "../libafanasy/host.h"
#include "../libafanasy/render.h"

#include "../libafqt/name_afqt.h"
#include "../libafqt/qenvironment.h"

#include "qobject.h"
#include "res.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Object * RenderObject = NULL;

//####################### interrupt signal handler ####################################
#include <signal.h>
void sig_pipe(int signum)
{
   AFERROR("afanasy:: SIGPIPE:\n");
}
void sig_int(int signum)
{
   if( RenderObject) RenderObject->exitRender();
}
//#####################################################################################

int main(int argc, char *argv[])
{
   Py_InitializeEx(0);
   uint32_t env_flags = af::Environment::AppendPythonPath;
#ifdef WINNT
   if( argc <= 1 ) env_flags = env_flags | af::Environment::Verbose; // Verbose environment initialization
   signal( SIGINT,  sig_int);
   signal( SIGTERM, sig_int);
   signal( SIGSEGV, sig_int);
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
   af::Environment ENV( env_flags, argc, argv);

   if( !ENV.isValid())
   {
      AFERROR("main: Environment initialization failed.\n");
      exit(1);
   }

   afqt::init( ENV.getRenderWaitForConnected(), ENV.getRenderWaitForReadyRead(), ENV.getRenderWaitForBytesWritten());

   uint32_t state = af::Render::SOnline;

   std::string NIMBY = "-NIMBY";
   std::string nimby = "-nimby";
   ENV.addUsage( NIMBY, "Set initial state to 'NIMBY'.");
   ENV.addUsage( nimby, "Set initial state to 'nimby'.");
   if( ENV.hasArgument( NIMBY))
   {
      printf("Initial state set to 'NIMBY'\n");
      state = state | af::Render::SNIMBY;
   }
   else if( ENV.hasArgument( nimby))
   {
      printf("Initial state set to 'nimby'\n");
      state = state | af::Render::Snimby;
   }

   uint8_t priority = ENV.getPriority();

   std::string cmdArgName = "-cmd";
   ENV.addUsage( cmdArgName + " [command]", "Run command only, do not connect to server.");
   std::string command;
   ENV.getArgument( cmdArgName, command);

   bool checkResourcesMode = false;
   std::string checkResourcesModeCmdArg = "-res";
   ENV.addUsage( checkResourcesModeCmdArg, "Check host resources only and quit.");
   if( ENV.hasArgument( checkResourcesModeCmdArg)) checkResourcesMode = true;

   int retval = 0;

   afqt::QEnvironment QENV;
   if( false == QENV.isValid())
   {
      AFERROR("main: QEnvironment initialization failed.\n")
      exit(1);
   }

   if(( false == ENV.isHelpMode()) &&
      ( false == checkResourcesMode ))
   {
      QCoreApplication app( argc, argv);
      Object object( state, priority, command);
      RenderObject = &object;
      QObject::connect( &object, SIGNAL( exitApplication()), &app, SLOT( quit()));
      retval = app.exec();
      printf("main: QApplication::exec: returned status = %d\n", retval);
   }

   if( checkResourcesMode )
   {
      af::Host host;
      af::HostRes hostres;
      GetResources( host, hostres, true, true);
#ifdef WINNT
      Sleep(100);
#else
      sleep(1);
#endif
      GetResources( host, hostres, false, true);
      printf("\n");
      host.stdOut( true);
      hostres.stdOut( true);
   }

   af::destroy();
   Py_Finalize();

   return retval;
}
