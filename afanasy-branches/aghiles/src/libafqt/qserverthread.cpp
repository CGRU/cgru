#include "qserverthread.h"
#include "name_afqt.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afqt;

QServerThread::QServerThread( int socketDescriptor, QObject * parent, bool (*handler_ptr)( QTcpSocket *, af::Msg *)):
   QThread( parent),
   sd( socketDescriptor),
   recvMessage_handler_ptr( handler_ptr)
{
AFINFO("QServerThread::QServerThread() \n");
}

QServerThread::~QServerThread()
{
AFINFO("QServerThread::~QServerThread() \n");
}

void QServerThread::run()
{
   QTcpSocket qsocket;
   if( qsocket.setSocketDescriptor( sd) == false)
   {
      AFERROR("QThreadServer::run: Can't set socket descriptor.\n");
      return;
   }

   af::Msg* msg = new af::Msg;
   if( afqt::recvMessage( &qsocket, msg))
   {
      if( recvMessage_handler_ptr == NULL )
      {
         emit newmsg( msg);
      }
      else
      {
         if( recvMessage_handler_ptr( &qsocket, msg) == false)
         {
            emit newmsg( msg);
         }
      }
   }
   qsocket.disconnectFromHost();
   if( qsocket.state() != QAbstractSocket::UnconnectedState ) qsocket.waitForDisconnected();
}
