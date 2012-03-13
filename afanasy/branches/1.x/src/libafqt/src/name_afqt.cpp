#include "name_afqt.h"

#include <QtNetwork/QHostAddress>

#include <address.h>
#include <environment.h>
#include <msg.h>

//#define AFOUTPUT
//#undef AFOUTPUT
#include <macrooutput.h>

int WAITFORCONNECTED;
int WAITFORREADYREAD;
int WAITFORBYTESWRITTEN;

void afqt::init( int waitforconnected, int waitforreadyread, int waitforbyteswritten)
{
   WAITFORCONNECTED    = waitforconnected;
   WAITFORREADYREAD    = waitforreadyread;
   WAITFORBYTESWRITTEN = waitforbyteswritten;
}

bool afqt::connectAfanasy( QTcpSocket * qSocket)
{
   if( qSocket->state() != QAbstractSocket::ConnectedState)
   {
#ifdef AFOUTPUT
printf("afqt::connectAfanasy: %s : %d\n", af::Environment::getAfServerQHostAddress()->toString().toUtf8().data(), af::Environment::getServerPort());
#endif
      qSocket->connectToHost( *(af::Environment::getAfServerQHostAddress()), af::Environment::getServerPort());
      if( qSocket->waitForConnected( WAITFORCONNECTED) == false)
      {
         AFINFO("qtnet::connectAfanasy: can't connect to server.\n");
         return false;
      }
   }
   else
   {
      AFINFO("qtcom::connectAfanasy: QTcpSocket seems to be already connected.\n");
   }
   return true;
}

bool afqt::connect( const af::Address * address, QTcpSocket * qSocket)
{
   if( qSocket->state() != QAbstractSocket::ConnectedState)
   {
      qSocket->connectToHost( QHostAddress( ntohl(address->IP4())), address->port());
      if( qSocket->waitForConnected( WAITFORCONNECTED) == false)
      {
         AFERROR("qtnet::connect: Can't connect to address ");
         address->stdOut(); printf("\n");
         return false;
      }
   }
   else
   {
      AFERROR("qtcom::connect: Socket seems to be already connected.\n");
   }
   return true;
}

bool afqt::sendMessage( QTcpSocket * qSocket, const af::Msg * msg)
{
#ifdef AFOUTPUT
printf("afqt::sendMessage: "); msg->stdOut();
#endif
//
// sending message to server
   int result = qSocket->write( msg->buffer(), msg->writeSize());
   qSocket->waitForBytesWritten( WAITFORBYTESWRITTEN);
   if( result < msg->writeSize())
   {
      AFERRAR("qtcom::sendMessage: sending writing message failed, result < msg.writeSize() ( %d < %d )\n", result, msg->writeSize());
      return false;
   }
   return true;
}

bool afqt::recvMessage( QTcpSocket * qSocket, af::Msg * msg)
{
   char * buffer = msg->buffer();
   int bytes = readdata( qSocket, buffer, af::Msg::SizeHeader, af::Msg::SizeBuffer);
   if( bytes < af::Msg::SizeHeader)
   {
      AFERROR("qtcom::recvMessage: read message header.\n");
      return false;
   }

   if( msg->readHeader( bytes) == false )
   {
      AFERROR("afqt::recvMessage: constructing message header failed.\n");
      return false;
   }

   int msgtype = msg->type();
   int datalen = msg->int32();

   if( msgtype >= af::Msg::TDATA)
   {
      buffer = msg->buffer(); // buffer may be changed to fit new size
      int readlen = datalen - bytes + af::Msg::SizeHeader;
      if( readlen > 0)
      {
         bytes = readdata( qSocket, buffer+bytes, readlen, readlen);
         if( bytes < readlen)
         {
            AFERROR("qtcom::recvMessage: read message data.\n");
            msg->setInvalid();
            return false;
         }
      }
   }
   return true;
}

int afqt::readdata( QTcpSocket * qSocket, char* data, int len_min, int len_max)
{
   AFINFA("qtcom::readdata: trying to recieve %d bytes.\n", len_min);
   int bytes = 0;
   while( bytes < len_min)
   {
      qSocket->waitForReadyRead( WAITFORREADYREAD);
      int r = qSocket->read( data+bytes, len_max);
      if( r < 0)
      {
         AFERRPE("qtcom::readdata::read");
         return 0;
      }
      if( r == 0)
      {
         AFERROR("qtcom::readdata:: read 0 bytes.\n");
         return 0;
      }
      bytes += r;
      AFINFA("qtcom::readdata::read %d bytes.\n", r);
   }
   return bytes;
}
