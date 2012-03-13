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
#ifdef MACOSX
      listening = listen( QHostAddress::Any);
#else
//      listening = listen( QHostAddress::Any, port);
      listening = listen( QHostAddress::AnyIPv6, port);
#endif
      if( listening)
      {
         port = serverPort();
         printf("QServer::QServer: Listening \"%s\" port %d ...\n", serverAddress().toString().toUtf8().data(), port);
         if( false == af::Environment::setClientPort(port))
         {
            AFERROR("QServer::QServer: Can't init with invalid address.\n");
            return;
         }
         break;
      }
      else
      {
         AFINFA("QServer::QServer: can't listen port %d (address may be already in use).\n", port);
      }
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
