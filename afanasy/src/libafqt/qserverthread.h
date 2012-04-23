#pragma once

#include "../libafanasy/msg.h"

#include "name_afqt.h"

#include <QtNetwork/QTcpSocket>
#include <QtCore/QThread>

class afqt::QServerThread : public QThread
{
   Q_OBJECT
public:
   QServerThread( int socketDescriptor, QObject * parent,  bool (*handler_ptr)( QTcpSocket *, af::Msg *) = NULL);
   ~QServerThread();

protected:
   void run();

signals:
   void newmsg( af::Msg *msg);

private:
   int sd;

   bool (*recvMessage_handler_ptr)( QTcpSocket *, af::Msg *);
};
