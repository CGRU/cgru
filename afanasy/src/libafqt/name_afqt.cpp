#include "name_afqt.h"

#include <QtNetwork/QHostAddress>

#include "../libafanasy/address.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "../libafqt/qenvironment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

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
printf("afqt::connectAfanasy: %s : %d\n", afqt::QEnvironment::getAfServerQHostAddress().toString().toUtf8().data(), af::Environment::getServerPort());
#endif
      qSocket->connectToHost( afqt::QEnvironment::getAfServerQHostAddress(), af::Environment::getServerPort());
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

bool afqt::connect( const af::Address & address, QTcpSocket * qSocket)
{
   if( qSocket->state() != QAbstractSocket::ConnectedState)
   {
//      QHostAddress addr;
//      address->setQAddress( addr);
//      qSocket->connectToHost( addr, address->getPortHBO());
      qSocket->connectToHost( afqt::toQAddress( address), address.getPortHBO());
      if( qSocket->waitForConnected( WAITFORCONNECTED) == false)
      {
         AFERROR("afqt::connect: Can't connect to address:")
         printf(" %s", address.v_generateInfoString().c_str());
         printf(" Q = %s", afqt::toQAddress( address).toString().toUtf8().data());
         printf("\n");
         return false;
      }
   }
   else
   {
      AFERROR("afqt::connect: Socket seems to be already connected.\n")
   }
   return true;
}

const QHostAddress afqt::toQAddress( const af::Address & address)
{
   QHostAddress qaddr;
   switch( address.getFamily())
   {
      case af::Address::IPv4:
      {
         uint32_t ipv4;
         memcpy( &ipv4, address.getAddrData(), 4);
         qaddr.setAddress( htonl(ipv4));
         break;
      }
      case af::Address::IPv6:
         qaddr.setAddress( (quint8*)(address.getAddrData()));
         break;
      default:
         AFERROR("Address::setQAddress: Unknown address family.\n");
   }
   return qaddr;
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

   msg->readHeader( bytes);

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

const QString afqt::stoq( const std::string & str)
{
//   return QString::fromUtf8( str.c_str());
   return QString::fromUtf8( str.data(), str.size());
}

const QString afqt::dtoq( const char * data, int size)
{
   if( size >= 0 ) return QString::fromUtf8( data, size);
   else return QString::fromUtf8( data);
}

const std::string afqt::qtos( const QString & str)
{
   return std::string( str.toUtf8().data());
}

const QString afqt::time2Qstr( time_t time_sec)
{
   return stoq( af::time2str(time_sec));
}
