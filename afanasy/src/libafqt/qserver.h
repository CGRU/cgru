#pragma once

#include "../libafanasy/msg.h"

#include "name_afqt.h"

#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtCore/QThread>

/// Qt Server thread class.
/** This class initialize QServer in a different thread.
*** Try to listen client port, defined in Environment in constructor.
*** If client port is busy, QServer increment it and try to listen next one.
*** So QServer will try to listen every port from
*** \c ENV.clientPort() to \c (ENV.clientPort()+AFADDR::MAXPORTS) .
**/

class afqt::QServer : public QTcpServer
{
   Q_OBJECT
public:
   QServer( QObject *parent);
   ~QServer();    ///< Dectructor for safety thread exit.
   void run();          ///< QThread.run() function override.
   inline bool isInitialized() const { return init;} ///< Return \c true if thread sucessfully initialized.
   inline void set_recvMessage_handler( bool (*handler_ptr)( QTcpSocket *, af::Msg *))
      { recvMessage_handler_ptr = handler_ptr;}

signals:
   void newMsg( af::Msg *msg); ///< Emitted when new message recieved.

private slots:
   void newmsg( af::Msg *msg);

protected:
   void incomingConnection( int sd);

private:
   bool (*recvMessage_handler_ptr)( QTcpSocket *, af::Msg *);
   QServerThread *thread;
   bool init;                          ///< Stores \c thue if initialization passed successfilly.
};
