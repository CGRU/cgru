#pragma once

#include "../libafanasy/name_af.h"

#include "name_afqt.h"
#include "qthreadclient.h"

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>

/// Class that can send messages in a different thread.
/** send( Msg *msg) function will write message into a buffer and execute a thread to send it,
*** or wake a thread if already exists one.
*** After sending that thead will sleep, until send function will wake it up.
**/
class afqt::QThreadClientSend : public afqt::QThreadClient
{
   Q_OBJECT
public:
   QThreadClientSend( QObject * parent, int NumConnLost = 1 );
   ~QThreadClientSend();      ///< Dectructor for safety thread exit.

   virtual void run();        ///< QThread.run() method override.
   void send( af::Msg * msg);    ///< Send message function.

protected:
};
