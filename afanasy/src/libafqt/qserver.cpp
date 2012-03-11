#include "qserver.h"

#include "../libafanasy/environment.h"

#include "name_afqt.h"
#include "qserverthread.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afqt;

QServer::QServer( QObject *parent):
   QTcpServer( parent),
   recvMessage_handler_ptr( NULL),
   thread( NULL),
   init( false)
{
   int port = af::Environment::getClientPort();
   int maxports = 65535-port;
   for( int i = 0; i <= maxports; i++)
   {
      bool listening = false;
      int port_to_listen = port;
#ifdef MACOSX
      port_to_listen = 0; // Let system choose port automatically
#endif
#ifndef WINNT
      listening = listen( QHostAddress::AnyIPv6, port_to_listen);
#endif
      if( false == listening ) listening = listen( QHostAddress::Any, port_to_listen);
      if( listening)
      {
         port = serverPort();
         printf("QServer::QServer: Listening \"%s\" port %d ...\n", serverAddress().toString().toUtf8().data(), port);
         af::Environment::setClientPort(port);
         break;
      }
      else
      {
         AFINFA("QServer::QServer: can't listen port %d (address may be already in use).\n", port);
      }
#ifdef MACOSX
      break; // Do not probe next port on MAC, system must choose port autamatically
#endif
      port++;
   }
   if( isListening())
   {
      init = true;
   }
   else
   {
      AFERROR("QServer::QServer: Not listening.\n");
   }
}

QServer::~QServer()
{
AFINFO("QServer::~QServer() \n");
}

void QServer::incomingConnection( int sd)
{
   thread = new QServerThread( sd, this, recvMessage_handler_ptr);
   connect( thread, SIGNAL( finished()         ), thread, SLOT( deleteLater() )     );
   connect( thread, SIGNAL( newmsg( af::Msg * )), this,   SLOT( newmsg( af::Msg * )));
   thread->start();
}

void QServer::newmsg( af::Msg *msg)
{
   emit newMsg( msg);
}
