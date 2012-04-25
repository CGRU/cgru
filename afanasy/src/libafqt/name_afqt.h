#pragma once

#include "../libafanasy/name_af.h"

#include <QtNetwork/QTcpSocket>

namespace afqt
{
/// Init environment variables.
   void init( int waitforconnected, int waitforreadyread, int waitforbyteswritten);

   class QEnvironment;

/// Connect to Afanasy server.
   bool connectAfanasy( QTcpSocket * qSocket);

/// Construct QHostAddress from af::Address.
   const QHostAddress toQAddress( const af::Address & address);

/// Connect to some adress.
   bool connect( const af::Address & address, QTcpSocket * qSocket);

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

   class QMsgQueue;

   class QThreadClient;
   class QThreadClientSend;
   class QThreadClientUp;

   class QServer;
   class QServerThread;

   const QString stoq( const std::string & str);
   const QString dtoq( const char * data, int size = -1);
   const std::string qtos( const QString & str);
   const QString time2Qstr( time_t time_sec = time( NULL));
}
