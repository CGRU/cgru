#pragma once

#include "../libafanasy/name_af.h"

#include "name_afqt.h"
#include "qmsgqueue.h"

#include <QtCore/QThread>

/// Class that can send messages in a different thread.
class afqt::QThreadClient : public QThread
{
   Q_OBJECT
public:
   QThreadClient( QObject * parent, int NumConnLost = 1 );
   ~QThreadClient();      ///< Dectructor for safety thread exit.

signals:
   void newMsg( af::Msg *msg);  ///< This signal will be emitted if server produced an answer.
   void connectionLost();       ///< When can't connect to address.

protected:
   afqt::QMsgQueue queue;

   void sendMessage( af::Msg * msg, QTcpSocket * socket); ///< Send message, stored in poiter.
   int numconnlost;   ///< Number of times to fail to connect to produce signal \c connectionLost.
   int connlostcount; ///< Current numder of fails to connect.
};
