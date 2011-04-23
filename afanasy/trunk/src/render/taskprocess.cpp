#include "taskprocess.h"

#include <QtCore/QDir>

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mclistenaddress.h"
#include "../libafanasy/msgclasses/mctaskoutput.h"
#include "../libafanasy/msgclasses/mctaskup.h"

#include "../libafqt/qmsg.h"
#include "../libafqt/qthreadclientsend.h"

#include "renderhost.h"
#include "parserhost.h"

extern afqt::QThreadClientSend * pCLIENT;
extern RenderHost * pRENDER;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

TaskProcess::TaskProcess( QObject * parent, af::TaskExec * taskExec, int runningtasks):
   ChildProcess( parent),
   exec( taskExec),
   service( *taskExec),
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

   QString command = afqt::stoq( service.getCommand());
   QString wdir    = afqt::stoq( service.getWDir());

   parser = new ParserHost( exec->getParserType(), exec->getFramesNum());//, "AF_PROGRESS %d");

   // Process task working directory:
   if( false == wdir.isEmpty())
   {
#ifdef WINNT
      if( wdir.startsWith("/"))
      {
         AFERRAR("Working directory starts with '/':\n\%s\n\May be it is not translated from unix?", wdir.toUtf8().data())
      }
      else if( wdir.startsWith("\\\\"))
      {
         AFERRAR("Working directory starts with '\\\\':\n\%s\n\UNC path can't be current. May be incorrect translation?", wdir.toUtf8().data())
      }
      else
#endif
      if( false == QDir( wdir).exists())
      {
         AFERRAR("Working directory does not exists:\n\%s", wdir.toUtf8().data())
      }
      else
      {
         setWorkingDirectory( wdir);
      }
   }

   printf("\nStarting[%d]: ", runningtasks); exec->stdOut( false);

#ifdef WINNT
//   args << "/c" << command;
//   args << "/c" << command.replace("\"","\"\"\"");
//   start( "cmd.exe", args, QIODevice::ReadWrite);
   start( QString("cmd.exe /c %1").arg(command), QIODevice::ReadWrite);
//   start( command, QIODevice::ReadWrite);
   setupChildProcess();
#else
   QStringList args;
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
   if( parser  != NULL  ) delete parser;

   AFINFO("TaskProcess:~TaskProcess()\n");
}

void TaskProcess::p_finished( int exitCode, QProcess::ExitStatus exitStatus)
{
   if( update_status == 0 ) return;
   printf("\nFinished: "); exec->stdOut( false);

   p_readyRead();

   if(( exitStatus != QProcess::NormalExit ) || ( stop_time != 0 ))
   {
      printf("Task terminated/killed.\n");
      if( update_status != af::TaskExec::UPFinishedParserError )
         update_status = af::TaskExec::UPFinishedCrash;
   }
   else
   {
      if( exitCode != 0)
      {
         update_status = af::TaskExec::UPFinishedError;
         printf("Error: exitcode = %d.\n", exitCode);
      }
      else if( parser->isBadResult())
      {
         update_status = af::TaskExec::UPFinishedParserBadResult;
         printf("Error: Bad result from parser (exitcode = %d).\n", exitCode);
      }
      else
      {
         update_status = af::TaskExec::UPFinishedSuccess;
         printf("Success: exitcode = %d.\n", exitCode);
      }
   }
   printf("\n");
   sendTaskSate();
}

void TaskProcess::p_readyRead()
{
   std::string output = readAll().data();
   if( output.size() == 0 ) return;
   parser->read( output);

   if( exec->getListenAddressesNum())
   {
#ifdef AFOUTPUT
printf("Sending output to addresses:");
#endif
      af::MCTaskOutput mctaskoutput( pRENDER->getName(), exec->getJobId(), exec->getBlockNum(), exec->getTaskNum(), output.size(), output.data());
      const std::list<af::Address> * addresses = exec->getListenAddresses();
      for( std::list<af::Address>::const_iterator it = addresses->begin(); it != addresses->end(); it++)
      {
#ifdef AFOUTPUT
printf(" ");(*it).stdOut();
#endif
         if( pCLIENT) pCLIENT->send( new afqt::QMsg( af::Msg::TTaskOutput, &mctaskoutput, false, &(*it)));
      }
#ifdef AFOUTPUT
printf("\n");
#endif
   }

   if( parser->hasWarning() && (update_status != af::TaskExec::UPWarning) && (update_status != af::TaskExec::UPFinishedParserError))
   {
      printf("Warning: Parser notification.\n");
      update_status = af::TaskExec::UPWarning;
   }
   if( parser->hasError() && ( stop_time == 0 ))
   {
      printf("Error: Bad result from parser. Stopping task.\n");
      update_status = af::TaskExec::UPFinishedParserError;
      stop();
   }
}

void TaskProcess::sendTaskSate()
{
   if( pCLIENT == NULL ) return;
   if( update_status == 0 ) return;

   int    type = af::Msg::TTaskUpdatePercent;
   bool   toRecieve = false;
   char * stdout_data = NULL;
   int    stdout_size = 0;

   if(( update_status != af::TaskExec::UPPercent ) &&
      ( update_status != af::TaskExec::UPWarning ))
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

   // Store the time when task was asked to be stopped (was asked first time)
   if( stop_time == 0 ) stop_time = time(NULL);

   // Return if task is not running
   if( pid() == 0 ) return;

   // Trying to terminate() first, and only if no response after some time, then perform kill()
   terminate();
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
         msg.setString("Render: Silence...");
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
      msg.setString("Render: parser is NULL.");
   }
}
