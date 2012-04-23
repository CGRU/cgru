#pragma once

#include "../libafqt/qserver.h"
#include "../libafqt/qthreadclientsend.h"
#include "../libafqt/qthreadclientup.h"

#include "monitorhost.h"

#include <QtGui/QMainWindow>

class ListNodes;
class ListUsers;
class ListJobs;
class ListRenders;

class MonitorWindow : public QMainWindow
{
   Q_OBJECT

public:
   MonitorWindow();
   ~MonitorWindow();

   inline bool isInitialized() const { return initialized; }
   inline bool isConnected()   const { return connected;   }

   inline void sendMsg( af::Msg * msg) { qthreadClientSend->send( msg); }
   inline int  getId() const { return monitor->getId(); }

   inline int getUsersSelectionCount() const { return usersSelectionCount; }
   inline const QList<int> * getUsersSelectionIds() const { return &usersSelectionIds; }

   enum MType{
      MTUsers,
      MTJobs,
      MTRenders,

      MTLast
   };

signals:
   void stop();

protected:
   void closeEvent( QCloseEvent * event);

private slots:
   void sendRegister();
   void caseMessage( af::Msg* msg);
   void connectionLost();

   void usersSelectionChanged();

private:
   bool connected;
   void connectionEstablished();

   QString username;
   QString hostname;
   MonitorHost *monitor;

   afqt::QThreadClientUp   *qthreadClientUp;
   afqt::QThreadClientSend *qthreadClientSend;
   afqt::QServer           *qServer;

   ListNodes   *   nodesList[MTLast];
   ListUsers   *   usersList;
   ListJobs    *    jobsList;
   ListRenders * rendersList;

   int usersSelectionCount;
   QList<int> usersSelectionIds;

   bool initialized;
};
