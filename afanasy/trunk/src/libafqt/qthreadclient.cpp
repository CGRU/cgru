#include "qthreadclient.h"
#include "name_afqt.h"

#include "../libafanasy/address.h"

using namespace afqt;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

QThreadClient::QThreadClient( QObject * parent, int NumConnLost):
   QThread( parent),
   numconnlost( NumConnLost),
   connlostcount( 0)
{
   setObjectName("QObj_QThreadClient");
}

QThreadClient::~QThreadClient()
{
AFINFO("QThreadClient::~QThreadClient() \n");
}

void QThreadClient::sendMessage( QMsg * msg, QTcpSocket * socket)
{
   if( msg == NULL ) return;

#ifdef AFOUTPUT
printf("QThreadClient::sendMessage: "); msg->stdOut();
#endif

   bool connected = false;
   if( msg->getAddress() )
      connected = afqt::connect( msg->getAddress(), socket);
   else
      connected = afqt::connectAfanasy(   socket);

   if( connected )
   {
      bool send = false;
      {
         if( afqt::sendMessage( socket, msg)) send = true;
      }
      if( msg->getRecieving() && send)
      {
         af::Msg *answer = new af::Msg;
         if( afqt::recvMessage( socket, answer))
         {
            emit newMsg( answer);
         }
         else
         {
            AFERROR("QThreadClient::run: recieving message failed.\n");
            delete answer;
         }
      }
      socket->disconnectFromHost();
      if( socket->state() != QAbstractSocket::UnconnectedState ) socket->waitForDisconnected();
      connlostcount = 0;
   }
   else
   {
      if((numconnlost != 0) && (connlostcount <= numconnlost)) connlostcount++;
      if((numconnlost  > 1) && (connlostcount <= numconnlost))
      {
         AFERRAR("%s: Connection lost count = %d (of %d)\n", objectName().toUtf8().data(), connlostcount, numconnlost);
      }
      if( connlostcount == numconnlost )
      {
         AFERRAR("%s: Connection Lost !\n", objectName().toUtf8().data());
         if( msg->getAddress() )  emit connectionLost( new af::Address( msg->getAddress()));
         else                     emit connectionLost( NULL);
      }
   }
}
