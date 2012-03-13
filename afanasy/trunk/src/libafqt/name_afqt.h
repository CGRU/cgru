#pragma once

#include <QtNetwork/QTcpSocket>

#include "../libafanasy/name_af.h"

namespace afqt
{
/// Init environment variables.
   void init( int waitforconnected, int waitforreadyread, int waitforbyteswritten);

   class QEnvironment;

/// Connect to Afanasy server.
   bool connectAfanasy( QTcpSocket * qSocket);

/// Connect to some adress.
   bool connect( const af::Address * address, QTcpSocket * qSocket);

/// Sent message.
   bool sendMessage( QTcpSocket * qSocket, const af::Msg * msg);

/// Recieve message.
   bool recvMessage( QTcpSocket * qSocket, af::Msg * msg);

/// Try to read data with given length \c len to buffer \c data .
   int readdata( QTcpSocket * qSocket, char* data, int len_min, int len_max);

   class Attr;
   class AttrNumber;
   class AttrColor;
   class AttrRect;

   class QMsg;
   class QMsgQueue;

   class QThreadClient;
   class QThreadClientSend;
   class QThreadClientUp;

   class QServer;
   class QServerThread;
}
