#include "qthreadclientup.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

using namespace afqt;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

QThreadClientUp::QThreadClientUp( QObject * Parent, bool Blocking, int Seconds, int NumConnLost):
   QThreadClient( Parent, NumConnLost),
   seconds(  Seconds),
   blocking( Blocking),
   update_handler_ptr( NULL),
   toQuit( false)
{
   setObjectName("QObj_QThreadClientUp");
   AFINFA("QThreadClientUp::QThreadClientUp: blocking=%d, seconds=%d, numconnlost=%d", blocking, seconds, NumConnLost)
   if( ! blocking )
   {
      timer.setInterval( 1000 * seconds);
      connect( &timer, SIGNAL( timeout()), this, SLOT( send()));
   }
}

QThreadClientUp::~QThreadClientUp()
{
AFINFO("QThreadClientUp::~QThreadClientUp()")
   toQuit = true;
   timer.stop();
   cond.wakeOne();
   wait();
}

void QThreadClientUp::setInterval( const int Seconds)
{
   seconds = Seconds;
   if( ! blocking) timer.setInterval( Seconds * 1000 );
}

void QThreadClientUp::setUpMsg( af::Msg * msg)
{
   toQuit = false;

   queue.push( msg);

   if( false == blocking )
   {
      if( false == timer.isActive() )
      {
         send();
         timer.start();
      }
   }
   else
   {
      if( false == isRunning() ) start();
   }
}

void QThreadClientUp::send()
{
   if (!isRunning())
      start();
   else
      cond.wakeOne();
}

af::Msg * afqt::QThreadClientUp::getMessage()
{
   while( queue.getCount() > 1)
   {
      //printf("QThreadClientUp::getMessage(): queue.getCount() > 1\n");
      delete queue.pop();
   }
   return queue.getFirst();
}

void QThreadClientUp::run()
{
AFINFO("QThreadClientUp::run()")
   QTcpSocket socket;

   if( blocking)
      while( ! toQuit )
      {
         af::Msg * message = getMessage();
#ifdef AFOUTPUT
printf("QThreadClientUp::run: (blocking) "); message->stdOut();
#endif
         if( message )
         {
            if( update_handler_ptr != NULL ) message = update_handler_ptr( message);
            sendMessage( message, &socket);
         }
#ifdef WINNT
         Sleep( seconds*1000);
#else
         sleep( seconds);
#endif
      }
   else
      while( ! toQuit )
      {
         af::Msg * message = getMessage();
#ifdef AFOUTPUT
printf("QThreadClientUp::run: (non-blocking) "); message->stdOut();
#endif
         if( message )
         {
            if( update_handler_ptr != NULL ) message = update_handler_ptr( message);
            sendMessage( message, &socket);
         }

         {
            QMutexLocker locker(&mutex);
            cond.wait(&mutex);
         }
      }
}
