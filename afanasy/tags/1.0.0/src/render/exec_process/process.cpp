#include "process.h"
#if defined Q_OS_UNIX
   #include <signal.h>
#endif

MyProcess::MyProcess( QObject * parent, int tasktype, char *outputstr, int numframes):
   QProcess( parent),
   parser( NULL),
   output_str( outputstr),
   task_type( tasktype),
   frames( numframes),
   timer( this)
{
   connect( this, SIGNAL( started()), this, SLOT( s_started()));
   connect( this, SIGNAL( finished( int, QProcess::ExitStatus)), this, SLOT( s_finished( int, QProcess::ExitStatus)));
   connect( this, SIGNAL( error( QProcess::ProcessError)), this, SLOT( s_error( QProcess::ProcessError)));
//   connect( this, SIGNAL( readyReadStandardError()), this, SLOT( s_readyReadStandardError()));
//   connect( this, SIGNAL( readyReadStandardOutput()), this, SLOT( s_readyReadStandardOutput()));

   connect( this, SIGNAL( readyRead()), this, SLOT( s_readyRead()));
//   connect( this, SIGNAL( bytesWritten( qint64 )), this, SLOT( s_bytesWritten( qint64 )));

   timer.setInterval( 100);
//   connect( &timer, SIGNAL(timeout()), this, SLOT( s_readyRead()));
}

MyProcess::~MyProcess()
{
   if( parser != NULL ) delete parser;
}

void MyProcess::begin( const QString& program, const QStringList& arguments)
{
   QProcess::start( program, arguments,
                QIODevice::Truncate   |
//                QIODevice::Text       |
                QIODevice::Unbuffered |
                QIODevice::ReadOnly   );
}

void MyProcess::resetOpenMode( QIODevice::OpenMode openMode)
{
   setOpenMode ( openMode);
}

void MyProcess::s_started()
{
printf("MyProcess::s_started()\n");
   if( parser != NULL ) delete parser;
   parser = new Parser( task_type, frames, output_str);
   timer.start();
}

void MyProcess::s_readyRead()
{
//   QByteArray data = read( BUFFER_SIZE);
   QByteArray data = readAll();
//   QByteArray data = readLine();
//   printf(".%s.", data.data());
   fflush( stdout);

   parser->read( data.data(), data.size());
}

void MyProcess::s_readyReadStandardError()
{
   QByteArray data = QProcess::readAllStandardError();
printf("MyProcess::s_readyReadStandardError() size = %d \n", data.size());
   printf("\n");
   printf( data.data());
   printf("\n");
}

void MyProcess::s_readyReadStandardOutput()
{
   QByteArray data = readAllStandardOutput();
   int size = data.size();
   printf(".%s.", data.data());
}

void MyProcess::s_finished( int exitCode, QProcess::ExitStatus exitStatus)
{
printf("MyProcess::s_finished()\n");
printf( "exitCode = %d, ",exitCode);
   timer.stop();
switch( exitStatus)
{
case QProcess::NormalExit:
   printf( "ExitStatus = NormalExit\n");
   break;
case QProcess::CrashExit:
   printf( "ExitStatus = CrashExit\n");
   break;
}
   if( parser != NULL )
   {
      printf("\n################ Process Output : ###########################\n");
      fflush( stdout);
      int size;
      char *data = parser->getData( &size);
      ::write( 1, data, size);
//      ::write( 1, parser->getData( &size), size);
//      printf( parser->getData( &size));
      fflush( stdout);
      printf("\n################# size = %d ######################\n", size);
      fflush( stdout);
      delete parser;
   }
   parser = NULL;
}

void MyProcess::s_error( QProcess::ProcessError error)
{
   printf("MyProcess::s_error()\n");
   switch( error)
   {
      case QProcess::FailedToStart:
         printf( "error = FailedToStart\n");
         break;
      case QProcess::Crashed:
         printf( "error = Crashed\n");
         break;
      case QProcess::Timedout:
         printf( "error = Timedout\n");
         break;
      case QProcess::WriteError:
         printf( "error = WriteError\n");
         break;
      case QProcess::ReadError:
         printf( "error = ReadError\n");
         break;
      case QProcess::UnknownError:
         printf( "error = UnknownError\n");
         break;
      default:
         printf( "error = ???\n");
         break;
   }
}

void MyProcess::setupChildProcess()
{
#if defined Q_OS_UNIX
   ::setsid();
#endif
}

void MyProcess::kill()
{
   printf("MyProcess::kill()\n");
#if defined Q_OS_UNIX
   ::killpg(::getpgid(pid()),SIGKILL);
#endif
}

void MyProcess::terminate()
{
   printf("MyProcess::terminate()\n");
#if defined Q_OS_UNIX
   ::killpg(::getpgid(pid()),SIGTERM);
#endif
}
