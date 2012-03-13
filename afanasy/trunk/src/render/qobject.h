#pragma once

#include "../libafanasy/name_af.h"

#include "../libafqt/name_afqt.h"
#include "../libafqt/qserver.h"
#include "../libafqt/qthreadclientsend.h"
#include "../libafqt/qthreadclientup.h"

#include "childprocess.h"

class Parser;
class RenderHost;
class TaskProcess;

class Object : public QObject
{
   Q_OBJECT

public:
   Object( uint32_t State, uint8_t Priority);
   ~Object();

   inline bool initialized() const { return init;}

   bool requestAnswer( QTcpSocket *qsocket, af::Msg *msg);

   void exitRender();

public:
   static Object * OBJECT;

signals:
   void exitApplication();

private slots:
   void caseMessage( af::Msg *msg);
   void refresh();
   void connectionLost( af::Address * address);
   void connectionLostOutput( af::Address * address);

private:
   void runTask( af::Msg *msg);
   void setUpMsg( int type);

private:
   bool connected;

   RenderHost *render;
   QList<TaskProcess*> tasks;
   QTimer timer;

   afqt::QServer           *qServer;
   afqt::QThreadClientUp   *qthreadClientUp;
   afqt::QThreadClientSend *qthreadClientSend;
   afqt::QThreadClientSend *qthreadClientSendOutput;

   bool init;
   bool exiting;
   bool exitRequest;
};
