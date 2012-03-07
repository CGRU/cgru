#include "qobject.h"
#if 0
#include <QtCore/QCoreApplication>

#include "../libafanasy/environment.h"
#include "../libafanasy/taskexec.h"
#include "../libafanasy/msgclasses/mclistenaddress.h"
#include "../libafanasy/msgclasses/mctaskpos.h"
#include "../libafanasy/msgclasses/mctaskoutput.h"
#include "../libafanasy/msgclasses/mctaskup.h"

#include "../libafqt/qmsg.h"

#include "parserhost.h"
#include "renderhost.h"
#include "taskprocess.h"

afqt::QThreadClientSend * pCLIENT;
RenderHost * pRENDER;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Object * Object::OBJECT = NULL;

bool recvMessage_handler( QTcpSocket *qsocket, af::Msg *msg) { return Object::OBJECT->requestAnswer( qsocket, msg);}

Object::Object( uint32_t State, uint8_t Priority, const std::string & command):
   connected( false),
   cmdMode( false),
   render( NULL),
   timer( this),
   qServer( NULL),
   qthreadClientUp( NULL),
   qthreadClientSend( NULL),
   qthreadClientSendOutput( NULL),
   init(false ),
   exiting( false),
   exitRequest( false)
{
   pCLIENT = NULL;
   pRENDER = NULL;

   if( command.empty())
   {
      qServer = new afqt::QServer( this);
      qthreadClientUp = new afqt::QThreadClientUp( this, true, af::Environment::getRenderUpdateSec(), af::Environment::getRenderConnectRetries());
      qthreadClientSend = new afqt::QThreadClientSend( this, af::Environment::getRenderConnectRetries());
      qthreadClientSendOutput = new afqt::QThreadClientSend( this, af::Environment::getRenderConnectRetries());
      if( qServer->isInitialized() == false )
      {
         AFERROR("Object::Object: Server initialization failed.")
         return;
      }
      qServer->set_recvMessage_handler( recvMessage_handler);
//      qthreadClientUp->set_update_handler( update_handler_ptr);
      //
      // connect network objects
      connect( qServer,           SIGNAL( newMsg( af::Msg*)), this, SLOT( caseMessage( af::Msg*) ));
      connect( qthreadClientSend, SIGNAL( newMsg( af::Msg*)), this, SLOT( caseMessage( af::Msg*) ));
      connect( qthreadClientUp,   SIGNAL( newMsg( af::Msg*)), this, SLOT( caseMessage( af::Msg*) ));
      connect( qthreadClientUp,         SIGNAL( connectionLost( af::Address*)), this, SLOT( connectionLost( af::Address*)));
      connect( qthreadClientSendOutput, SIGNAL( connectionLost( af::Address*)), this, SLOT( connectionLostOutput( af::Address*)));
   }
   else
   {
      cmdMode = true;
      tasks.append( new TaskProcess( /*this,*/ new af::TaskExec( command)/*, 0*/));
   }

   connect( &timer, SIGNAL( timeout()), this, SLOT( refresh()));
   timer.start( 1000);

   render = new RenderHost( State, Priority);
   init = true;
   render->stdOut();

   pCLIENT = qthreadClientSend;
   pRENDER = render;
   OBJECT = this;

   setUpMsg( af::Msg::TRenderRegister);
}

Object::~Object()
{
   if( false == exiting )
   {
      if( false == cmdMode )
      {
         qthreadClientUp->stop();
         qthreadClientSend->send( new afqt::QMsg( af::Msg::TRenderDeregister, render->getId()));
      }
   }
   for( int t = 0; t < tasks.size(); t++) delete tasks[t];
   if( render != NULL  ) delete render;
AFINFO("Object:~Object()")
}

void Object::connectionLost( af::Address * address)
{
   if( cmdMode ) return;
   if( connected == false ) return;
   connected = false;
//   render->setId( 0);

   for( int t = 0; t < tasks.size(); t++) tasks[t]->stop();

   setUpMsg( af::Msg::TRenderRegister);

   printf("Object: connection lost, connecting...\n");

}

void Object::connectionLostOutput( af::Address * address)
{
   if( cmdMode ) return;
   if( address == NULL)
   {
      AFERROR("Object::connectionLostOutput: Address == NULL.")
      return;
   }
   printf("Connection lost with listening address: "); address->stdOut(); printf("\n");

   int lasttasknum = -1;
   for( int t = 0; t < tasks.size(); t++) if( ((tasks[t])->m_taskexec)->removeListenAddress( address)) lasttasknum = t;
   if( lasttasknum != -1)
   {
      af::MCListenAddress mclass( af::MCListenAddress::FROMRENDER, address, tasks[lasttasknum]->m_taskexec->getJobId());
      qthreadClientSendOutput->send( new afqt::QMsg( af::Msg::TTaskListenOutput, &mclass));
   }

   delete address;
}

void Object::caseMessage( af::Msg *msg)
{
#ifdef AFOUTPUT
printf("Object::caseMessage: "); msg->stdOut();
#endif
   if( !connected)
   {
      connected = true;
      printf("Object: connected.\n");
   }
   if( msg == NULL)
   {
      AFERROR("Object::caseMessage: msg == NULL")
      return;
   }

   switch( msg->type())
   {
   case af::Msg::TRenderId:
   {
      int id = msg->int32();
      // Server sends back -1 id if a render with the same hostname already exists:
      if( id == -1)
      {
         AFERRAR("Render with this hostname '%s' already registered.", af::Environment::getHostName().c_str())
         exitRender();
      }
      // Render was trying to register (its id==0) and server has send id>0
      // This is the situation when client was sucessfully registered
      else if((id > 0) && (render->getId() == 0))
      {
//         render->setId( id);
         setUpMsg( af::Msg::TRenderUpdate);
      }
      // Server sends back zero id on any error
      // May be server was restarted and knows nothing about this render, so render must terister first
      else if ( render->getId() != id )
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
      exitRender();
      break;
   }
   case af::Msg::TClientRestartRequest:
   {
      exitRender();
      printf("Restart client request, executing command:\n%s\n", af::Environment::getRenderExec().c_str());
      QProcess::startDetached( afqt::stoq( af::Environment::getRenderExec()));
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
               if( mcaddr.toListen()) tasks[t]->m_taskexec->addListenAddress( mcaddr.getAddress());
               else                   tasks[t]->m_taskexec->removeListenAddress( mcaddr.getAddress());
               mcaddr.stdOut();
            }
         }
         else
         {
            if( tasks[t]->m_taskexec->getJobId() == mcaddr.getJobId())
            {
               if( mcaddr.toListen()) tasks[t]->m_taskexec->addListenAddress( mcaddr.getAddress());
               else                   tasks[t]->m_taskexec->removeListenAddress( mcaddr.getAddress());
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

void Object::runTask( af::Msg *msg)
{
   tasks.append( new TaskProcess( /*this,*/ new af::TaskExec( msg)/*, tasks.size()*/));
}

void Object::setUpMsg( int type)
{
   if( cmdMode ) return;
   afqt::QMsg * msg = new afqt::QMsg( type, render, true);
   qthreadClientUp->setUpMsg( msg);
}

bool Object::requestAnswer( QTcpSocket *qsocket, af::Msg *msg)
{
   switch( msg->type())
   {
      case af::Msg::TTaskOutputRequest:
      {
         af::MCTaskPos taskpos( msg);
         af::Msg answer;
         for( int t = 0; t < tasks.size(); t++) if( tasks[t]->is( taskpos))
         {
            tasks[t]->getOutput( answer);
            break;
         }
         if( false == answer.isNull()) afqt::sendMessage( qsocket, &answer);
         return true;
      }
   }
   return false;
}

void Object::refresh()
{
//printf("Object::refresh(): tasks.size() == %d\n", tasks.size());
   for( int t = 0; t < tasks.size(); t++)
   {
      tasks[t]->refresh();
      if( tasks[t]->isZombie())
      {
         delete tasks[t];
         tasks.removeAt( t);
      }
   }

   if( exitRequest && ( exiting == false ))
   {
      printf("Object::refresh(): Exit requested. Finishing %d task(s).\n", tasks.size());
      for( int t = 0; t < tasks.size(); t++) tasks[t]->stop( true /* No update needed*/);
      exiting = true;
      if( false == cmdMode )
      {
         qthreadClientUp->stop();
         qthreadClientSend->send( new afqt::QMsg( af::Msg::TRenderDeregister, render->getId()));
      }
   }

   if( exiting && ( tasks.size() == 0 ))
      emit exitApplication();
}

void Object::exitRender()
{
   exitRequest = true;
}
#endif
