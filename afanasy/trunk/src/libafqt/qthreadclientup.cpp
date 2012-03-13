#include "qthreadclientup.h"

#include "../libafanasy/environment.h"

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
   AFINFA("QThreadClientUp::QThreadClientUp: blocking=%d, seconds=%d, numconnlost=%d\n", blocking, seconds, NumConnLost);
   if( ! blocking )
   {
      timer.setInterval( 1000 * seconds);
      connect( &timer, SIGNAL( timeout()), this, SLOT( send()));
   }
}

QThreadClientUp::~QThreadClientUp()
{
AFINFO("QThreadClientUp::~QThreadClientUp() \n");
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

void QThreadClientUp::setUpMsg( QMsg * msg)
{
   toQuit = false;

   queue.push( msg);

   if( ! blocking )
   {
      if( ! timer.isActive() )
      {
         send();
         timer.start();
      }
   }
   else
   {
      if( ! isRunning() )            start();
   }
}

void QThreadClientUp::send()
{
   if (!isRunning())
      start();
   else
      cond.wakeOne();
}

QMsg * afqt::QThreadClientUp::getMessage()
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
AFINFO("QThreadClientUp::run() \n");
   QTcpSocket socket;

   if( blocking)
      while( ! toQuit )
      {
         QMsg * message = getMessage();
#ifdef AFOUTPUT
printf("QThreadClientUp::run: (blocking) "); message->stdOut();
#endif
         if( message )
         {
            if( update_handler_ptr != NULL ) update_handler_ptr( message);
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
         QMsg * message = getMessage();
#ifdef AFOUTPUT
printf("QThreadClientUp::run: (non-blocking) "); message->stdOut();
#endif
         if( message )
         {
            if( update_handler_ptr != NULL ) update_handler_ptr( message);
            sendMessage( message, &socket);
         }

         {
            QMutexLocker locker(&mutex);
            cond.wait(&mutex);
         }
      }
}

