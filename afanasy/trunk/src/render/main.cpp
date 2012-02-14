//#include <QtCore/QCoreApplication>

//#include <Python.h>

#include "../libafanasy/environment.h"
#include "../libafanasy/dlThread.h"
#include "../libafanasy/host.h"
#include "../libafanasy/render.h"

//#include "../libafqt/name_afqt.h"
//#include "../libafqt/qenvironment.h"

//#include "qobject.h"
#include "res.h"
#include "renderhost.h"
#include "rglobal.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

extern bool AFRunning;

//####################### interrupt signal handler ####################################
#include <signal.h>
void sig_pipe(int signum)
{
    AFERROR("SIGPIPE:");
}
void sig_int(int signum)
{
    AFRunning = false;
}
//#####################################################################################

// Functions:
void threadAcceptClient( void * i_arg );
void msgCase( af::Msg * msg);

int main(int argc, char *argv[])
{
    Py_InitializeEx(0);

   // Set signals handlers:
#ifdef WINNT
    signal( SIGINT,  sig_int);
    signal( SIGTERM, sig_int);
    signal( SIGSEGV, sig_int);
#else
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

    // Initialize environment and try to append python path:
    af::Environment ENV( af::Environment::AppendPythonPath, argc, argv);
    if( !ENV.isValid())
    {
        AFERROR("main: Environment initialization failed.\n");
        exit(1);
    }

    // Fill command arguments:
    std::string nimby = "-nimby";
    ENV.addUsage( nimby, "Set initial state to 'nimby'.");
    std::string NIMBY = "-NIMBY";
    ENV.addUsage( NIMBY, "Set initial state to 'NIMBY'.");
    std::string cmdArgName = "-cmd";
    ENV.addUsage( cmdArgName + " [command]", "Run command only, do not connect to server.");
    std::string command;
    ENV.getArgument( cmdArgName, command);
    std::string checkResourcesModeCmdArg = "-res";
    ENV.addUsage( checkResourcesModeCmdArg, "Check host resources only and quit.");
    ENV.addUsage("-noIPv6", "Disable IPv6.");
    // Help mode, usage is alredy printed, exiting:
    if( ENV.isHelpMode() )
        return 0;

    // Check resources and exit:
    if( ENV.hasArgument( checkResourcesModeCmdArg) )
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
        Py_Finalize();
        return 0;
    }

    // Some initial render properties:
    uint32_t state = af::Render::SOnline;
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

    RGlobal rGlobal;
    RenderHost render( state, priority);

    DlThread ServerAccept;
    ServerAccept.Start( &threadAcceptClient, NULL);

    while( AFRunning)
    {
        af::Msg * msg = RGlobal::acceptTry();
        if( msg ) msgCase( msg);
        sleep(1);
    }
//    QCoreApplication app( argc, argv);
//    Object object( state, priority, command);
//    RenderObject = &object;
//    QObject::connect( &object, SIGNAL( exitApplication()), &app, SLOT( quit()));
//    retval = app.exec();

    Py_Finalize();

    return 0;
}

void msgCase( af::Msg * msg)
{
#ifdef AFOUTPUT
printf("msgCase: "); msg->stdOut();
#endif
    if( msg == NULL)
    {
        return;
    }

    if( false == RGlobal::isConnected())
    {
        RGlobal::setConnected();
    }

    switch( msg->type())
    {
    case af::Msg::TRenderId:
    {
        int new_id = msg->int32();
      // Server sends back -1 id if a render with the same hostname already exists:
        if( new_id == -1)
        {
            AFERRAR("Render with this hostname '%s' already registered.", af::Environment::getHostName().c_str())
            AFRunning = false;
        }
        // Render was trying to register (its id==0) and server has send id>0
        // This is the situation when client was sucessfully registered
        else if((new_id > 0) && (render->getId() == 0))
        {
            render->setId( new_id);
//         setUpMsg( af::Msg::TRenderUpdate);
        }
        // Server sends back zero id on any error
        // May be server was restarted and knows nothing about this render, so render must register first
        else if ( render->getId() != new_id )
            connectionLost( NULL);
        break;
    }
    case af::Msg::TTask:
    {
        runTask( msg);
        break;
    }
/*   case af::Msg::TRenderTaskStdOutRequest:
   {
      break;
   }*/
    case af::Msg::TVersionMismatch:
    case af::Msg::TClientExitRequest:
    {
        AFRunning = false;
        break;
    }
    case af::Msg::TClientRestartRequest:
    {
        printf("Restart client request, executing command:\n%s\n", af::Environment::getRenderExec().c_str());
        QProcess::startDetached( afqt::stoq( af::Environment::getRenderExec()));
        AFRunning = false;
        break;
    }
/*   case af::Msg::TClientStartRequest:
   {
      printf("Start client request, executing command:\n%s\n", af::Environment::getRenderExec().c_str());
      QProcess::startDetached( afqt::stoq( af::Environment::getRenderExec()));
      break;
   }*/
   case af::Msg::TClientRebootRequest:
   {
      exitRender();
      printf("Reboot request, executing command:\n%s\n", af::Environment::getRenderCmdReboot().c_str());
      QProcess::startDetached( afqt::stoq( af::Environment::getRenderCmdReboot()));
      break;
   }
   case af::Msg::TClientShutdownRequest:
   {
      exitRender();
      printf("Shutdown request, executing command:\n%s\n", af::Environment::getRenderCmdShutdown().c_str());
      QProcess::startDetached( afqt::stoq( af::Environment::getRenderCmdShutdown()));
      break;
   }
   case af::Msg::TClientWOLSleepRequest:
   {
      printf("Sleep request, executing command:\n%s\n", af::Environment::getRenderCmdWolSleep().c_str());
      QProcess::startDetached( afqt::stoq( af::Environment::getRenderCmdWolSleep()));
      break;
   }
   case af::Msg::TRenderStopTask:
   {
      af::MCTaskPos taskpos( msg);
//printf("Msg::TRenderStopTask:\n"); taskpos.stdOut();
      for( int t = 0; t < tasks.size(); t++) if( tasks[t]->is( taskpos)) tasks[t]->stop();
   }
   case af::Msg::TRenderCloseTask:
   {
      af::MCTaskPos taskpos( msg);
//printf("Msg::TRenderCloseTask: [%d][%d][%d](%d)\n", taskpos.getJobId(), taskpos.getNumBlock(), taskpos.getNumTask(), taskpos.getNumber());
      for( int t = 0; t < tasks.size(); t++)
         if( tasks[t]->is( taskpos))
         {
            delete tasks[t];
            tasks.removeAt( t);
         }
      break;
   }
   case af::Msg::TTaskListenOutput:
   {
      af::MCListenAddress mcaddr( msg);
      for( int t = 0; t < tasks.size(); t++)
      {
         if( mcaddr.justTask())
         {
            if( tasks[t]->is( mcaddr.getJobId(), mcaddr.getNumBlock(), mcaddr.getNumTask(), 0))
            {
               if( mcaddr.toListen()) tasks[t]->exec->addListenAddress( mcaddr.getAddress());
               else                   tasks[t]->exec->removeListenAddress( mcaddr.getAddress());
               mcaddr.stdOut();
            }
         }
         else
         {
            if( tasks[t]->exec->getJobId() == mcaddr.getJobId())
            {
               if( mcaddr.toListen()) tasks[t]->exec->addListenAddress( mcaddr.getAddress());
               else                   tasks[t]->exec->removeListenAddress( mcaddr.getAddress());
               mcaddr.stdOut();
            }
         }
      }
      break;
   }
   default:
   {
      AFERROR("Object::caseMessage Unknown: message recieved.")
      msg->stdOut();
      break;
   }
   }

   delete msg;
}
