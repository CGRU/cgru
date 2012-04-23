#pragma once

#include "../libafanasy/name_af.h"

#include "name_afqt.h"
#include "qthreadclient.h"

#include <QtCore/QMutex>
#include <QtCore/QTimer>
#include <QtCore/QWaitCondition>

/// This thread send message to server every \c sec seconds.
/** This is usefull to update client information on server or request some data to update client.
**/
class afqt::QThreadClientUp : public afqt::QThreadClient
{
   Q_OBJECT
public:

   QThreadClientUp( QObject * Parent, bool Blocking, int Seconds, int NumConnLost);

   ~QThreadClientUp();                 ///< Dectructor for safety thread exit.

   virtual void run();                                ///< QThread.run() method override.

   void setUpMsg( af::Msg * msg);   ///< Set message for update (Msg::TNone - the default).

   void setInterval( const int Seconds);///< Set update interval.

   inline void stop() { toQuit = true; timer.stop();} ///< Stop updating.

   inline void set_update_handler( af::Msg * (*handler_ptr)( af::Msg *))
      { update_handler_ptr = handler_ptr;}

private:
   af::Msg * getMessage();
   QTimer timer;     ///< Qt timer pointer.
   QMutex mutex;
   QWaitCondition cond;
   int  seconds;
   bool blocking;
   af::Msg * (*update_handler_ptr)( af::Msg *);
   bool toQuit;

private slots:
   void send();
};
