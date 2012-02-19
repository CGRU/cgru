#include "qthreadclientsend.h"

#include "../libafanasy/msg.h"

using namespace afqt;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

QThreadClientSend::QThreadClientSend( QObject * parent, int NumConnLost):
   QThreadClient( parent, NumConnLost)
{
   setObjectName("QObj_QThreadClientSend");
   start();
}

QThreadClientSend::~QThreadClientSend()
{
   queue.push( NULL);
   wait();
AFINFO("QThreadClientSend::~QThreadClientSend()")
}

void QThreadClientSend::send( af::Msg * msg)
{
#ifdef AFOUTPUT
printf("QThreadClientSend::send: "); msg->stdOut();
#endif
   queue.push( msg);
}

void QThreadClientSend::run()
{
AFINFO("QThreadClientSend::run()")
   QTcpSocket socket;

   for(;;)
   {
      af::Msg * msg = queue.pop();
      if( msg != NULL)
      {
         sendMessage( msg, &socket);
         delete msg;
      }
      else return;
   }
}
