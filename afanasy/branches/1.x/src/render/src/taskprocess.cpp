#include "taskprocess.h"

// General definitions:
#include <afanasy.h>

// Common Afanasy library:
#include <environment.h>
#include <service.h>
#include <msgclasses/mclistenaddress.h>
#include <msgclasses/mctaskoutput.h>
#include <msgclasses/mctaskup.h>

// Afanasy Qt network library:
#include <qmsg.h>
#include <qthreadclientsend.h>

// Local:
#include "parserhost.h"
#include "renderhost.h"

extern afqt::QThreadClientSend * pCLIENT;
extern RenderHost * pRENDER;

//#define AFOUTPUT
//#undef AFOUTPUT
#include <macrooutput.h>

TaskProcess::TaskProcess( QObject * parent, af::TaskExec * taskExec, int runningtasks):
   ChildProcess( parent),
   exec( taskExec),
   service( NULL),
   parser( NULL),
   timer( this),
   update_status( af::TaskExec::UPPercent),
   stop_time( 0),
   zombie( false)
{
   setProcessChannelMode( QProcess::MergedChannels);
//
// connect process
   connect( this, SIGNAL( finished( int , QProcess::ExitStatus ) ),
            this, SLOT( p_finished( int , QProcess::ExitStatus ) ));
   connect( this, SIGNAL( readyRead() ),
            this, SLOT( p_readyRead() ));

//
// connect timer
   connect( &timer, SIGNAL( timeout() ), this, SLOT( sendTaskSate() ));
   timer.setInterval( af::Environment::getRenderUpdateTaskPeriod() * 1000 * (runningtasks+1));

   QString command = exec->getCmd();

   printf("\nStarting[%d]: ", runningtasks); exec->stdOut( true);
   if( exec->hasWDir()) setWorkingDirectory( exec->getWDir());

   service = new af::Service(
         exec->getServiceType(),
         exec->getCmd(),
         exec->getCmdView()
         );

   parser = new ParserHost( exec->getParserType(), exec->getFramesNum());//, "AF_PROGRESS %d");

   if( service)
   {
      if( exec->getCapCoeff())
      {
         command = service->applyCmdCapacity( exec->getCapCoeff());
         printf("   Capacity coefficient applied to command:\n%s\n", command.toUtf8().data());
      }
      if( exec->getMultiHostsNames().size() > 0)
      {
         command = service->applyCmdHosts( exec->getMultiHostsNames());
         printf("   Hosts names applied to command:\n%s\n", command.toUtf8().data());
      }
   }

   QStringList args;
#ifdef WINNT
   args << "/c" << command;
   start( "cmd.exe", args, QIODevice::ReadWrite);
   setupChildProcess();
#else
   args << "-c" << command;
   start( "/bin/bash", args, QIODevice::ReadWrite);
#endif

   timer.start();
}

TaskProcess::~TaskProcess()
{
   update_status = 0;//af::TaskExec::UPRenderDeregister;
   timer.stop();
   killProcess();

   if( exec    != NULL  ) delete exec;
   if( service != NULL  ) delete service;
   if( parser  != NULL  ) delete parser;

   AFINFO("TaskProcess:~TaskProcess()\n");
}

void TaskProcess::p_finished( int exitCode, QProcess::ExitStatus exitStatus)
{
   if( update_status == 0 ) return;
   printf("Finished: "); exec->stdOut( false);
   if( exitStatus == QProcess::NormalExit)
   {
      if( exitCode != 0)
      {
         update_status = af::TaskExec::UPFinishedError;
         printf("exitCode = %d != 0.\n", exitCode);
      }
      else
      {
         update_status = af::TaskExec::UPFinishedSuccess;
         printf("success. (exitCode=%d)\n", exitCode);
      }
   }
   else
   {
      update_status = af::TaskExec::UPFinishedCrash;
      printf("exitStatus != QProcess::NormalExit.\n");
   }
   printf("\n");
   sendTaskSate();
}

void TaskProcess::p_readyRead()
{
   if( parser == NULL) return;
   QByteArray output = readAll();
   parser->read( output.data(), output.size());

   if( exec->getListenAddressesNum() == 0 ) return;

#ifdef AFOUTPUT
printf("Sending output to addresses:");
#endif
   af::MCTaskOutput mctaskoutput( pRENDER->getName(), exec->getJobId(), exec->getBlockNum(), exec->getTaskNum(), output.size(), output.data());
   const std::list<af::Address*> * addresses = exec->getListenAddresses();
   for( std::list<af::Address*>::const_iterator it = addresses->begin(); it != addresses->end(); it++)
   {
#ifdef AFOUTPUT
printf(" ");(*it)->stdOut();
#endif
      pCLIENT->send( new afqt::QMsg( af::Msg::TTaskOutput, &mctaskoutput, false, *it));
   }
#ifdef AFOUTPUT
printf("\n");
#endif
}

void TaskProcess::sendTaskSate()
{
   if( update_status == 0 ) return;

   int    type = af::Msg::TTaskUpdatePercent;
   bool   toRecieve = false;
   char * stdout_data = NULL;
   int    stdout_size = 0;

   if( update_status != af::TaskExec::UPPercent )
   {
      timer.setInterval( 10 * af::Environment::getRenderUpdateTaskPeriod() * 1000);
      type = af::Msg::TTaskUpdateState;
      toRecieve = true;
      if( parser) stdout_data = parser->getData( &stdout_size);
   }

   int percent        = 0;
   int frame          = 0;
   int percentframe   = 0;

   if( parser)
   {
      percent        = parser->getPercent();
      frame          = parser->getFrame();
      percentframe   = parser->getPercentFrame();
   }

   af::MCTaskUp taskup(
                     pRENDER->getId(),

                     exec->getJobId(),
                     exec->getBlockNum(),
                     exec->getTaskNum(),
                     exec->getNumber(),

                     update_status,
                     percent,
                     frame,
                     percentframe,

                     stdout_size,
                     stdout_data
                  );
   afqt::QMsg * msg = new afqt::QMsg( type, &taskup, toRecieve);
//printf("TaskProcess::sendTaskSate:\n");msg->stdOut();printf("\n");
   pCLIENT->send( msg);
}

void TaskProcess::stop( bool noStatusUpdate)
{
   if( noStatusUpdate ) update_status = 0;
   if( pid() == 0 ) return;
   // Trying to terminate() first, and only if no response after some time, then perform kill()
   terminate();
//   QTimer::singleShot( 10000, this, SLOT( killProcess()));
   stop_time = time(NULL);
}

void TaskProcess::refresh()
{
   if( stop_time )
   {
      if( pid() == 0 ) zombie = true;
      else if( time( NULL) - stop_time > AFRENDER::TERMINATEWAITKILL ) killProcess();
   }
}

void TaskProcess::killProcess()
{
   if( pid() == 0 ) return;
   printf("KILLING NOT TERMINATED TASK.\n");
   kill();
}

void TaskProcess::getOutput( af::Msg & msg) const
{
   if( parser != NULL)
   {
      int size;
      char *data = parser->getData( &size);
      if( size > 0)
      {
         msg.setData( size, data);
      }
      else
      {
         QString str("Render: Silence...");
         msg.setString( str);
      }
/*
#ifdef AFOUTPUT
printf("\n######   Render::recieveMessage: case Msg::TRenderTaskStdOutRequest: ######\n");
fflush( stdout);
int ws = ::write( 1, data, size);
fflush( stdout);
printf("\n################################## size = %d ###########################\n", size);
fflush( stdout);
#endif
*/
   }
   else
   {
      QString str("Render: parser is NULL.");
      msg.setString( str);
   }
}
