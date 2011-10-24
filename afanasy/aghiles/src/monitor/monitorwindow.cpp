#include "monitorwindow.h"

#include <QtGui/QDockWidget>
#include <QtGui/QMenuBar>

#include "../libafanasy/environment.h"

#include "../libafqt/name_afqt.h"
#include "../libafqt/qmsg.h"

#include "listjobs.h"
#include "listusers.h"
#include "listrenders.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

MonitorWindow::MonitorWindow():
   connected(false),
   usersSelectionCount(0),
   initialized(false )
{
   qServer = new afqt::QServer( this);
   qthreadClientUp = new afqt::QThreadClientUp( this, false, af::Environment::getMonitorUpdatePeriod(), af::Environment::getMonitorConnectRetries());
   qthreadClientSend = new afqt::QThreadClientSend( this);
   if( qServer->isInitialized() == false )
   {
      AFERROR("MonitorWindow::MonitorWindow: Server initialization failed.\n");
      return;
   }
   username = afqt::stoq( af::Environment::getUserName());
   hostname = afqt::stoq( af::Environment::getHostName());
   setWindowTitle("MonitorWindow::" + username + "@" + hostname + ":(connecting...)");


   QMenu * viewMenu = menuBar()->addMenu("View");


   QDockWidget * dock;

   dock = new QDockWidget( "Renders", this);
   rendersList = new ListRenders( dock);
   dock->setWidget( rendersList);
   addDockWidget( Qt::LeftDockWidgetArea, dock);
   viewMenu->addAction( dock->toggleViewAction());

   dock = new QDockWidget( "Users", this);
   usersList = new ListUsers( dock);
   dock->setWidget( usersList);
   addDockWidget( Qt::RightDockWidgetArea, dock);
   viewMenu->addAction( dock->toggleViewAction());

   dock = new QDockWidget( "Jobs", this);
   jobsList = new ListJobs( dock);
   dock->setWidget( jobsList);
   addDockWidget( Qt::RightDockWidgetArea, dock);
   viewMenu->addAction( dock->toggleViewAction());

   nodesList[ MTRenders ] = rendersList;
   nodesList[ MTUsers   ] = usersList;
   nodesList[ MTJobs    ] = jobsList;

   connect( usersList,         SIGNAL( itemSelectionChanged()), this, SLOT( usersSelectionChanged()));

   connect( qServer,           SIGNAL( newMsg( af::Msg*)), this, SLOT( caseMessage( af::Msg*)));
   connect( qthreadClientUp,   SIGNAL( newMsg( af::Msg*)), this, SLOT( caseMessage( af::Msg*)));
   connect( qthreadClientSend, SIGNAL( newMsg( af::Msg*)), this, SLOT( caseMessage( af::Msg*)));
   connect( qthreadClientUp,   SIGNAL( connectionLost( af::Address*) ), this, SLOT( connectionLost( af::Address*)));
   connect( qthreadClientSend, SIGNAL( connectionLost( af::Address*) ), this, SLOT( connectionLost( af::Address*)));
   monitor = new MonitorHost();
   initialized = true;
   monitor->stdOut();
   sendRegister();
}

MonitorWindow::~MonitorWindow()
{
   if( monitor != NULL) delete monitor;
}

void MonitorWindow::closeEvent( QCloseEvent * event)
{
   qthreadClientSend->send( new afqt::QMsg( af::Msg::TMonitorDeregister, monitor->getId()));
   connected = false;
}

void MonitorWindow::sendRegister()
{
//printf("MonitorWindow::sendRegister()\n");
   afqt::QMsg * msg = new afqt::QMsg( af::Msg::TMonitorRegister, monitor, true);
   qthreadClientUp->setUpMsg( msg);
}

void MonitorWindow::connectionLost( af::Address* address)
{
   if( connected == false ) return;
   connected = false;

   monitor->setId( 0);
   sendRegister();
   printf("MonitorWindow::connectionLost(): connecting...\n");

   for( int t = 0; t < MTLast; t++) nodesList[t]->connectionLost();

   setWindowTitle("MonitorWindow::" + username + "@" + hostname + ":(connecting...)");
}

void MonitorWindow::connectionEstablished()
{
   connected = true;
   setWindowTitle( "MonitorWindow - " + afqt::stoq( af::Environment::getUserName()) + '@' + afqt::stoq( af::Environment::getHostName()) + ':' + QString::number(af::Environment::getClientPort()));

   for( int t = 0; t < MTLast; t++) nodesList[t]->connectionEstablished();
}

void MonitorWindow::caseMessage( af::Msg *msg)
{
//AFINFO("void MonitorWindow::caseMessage( Msg msg)\n");
   if( msg == NULL)
   {
      AFERROR("MonitorWindow::caseMessage: msg == NULL\n");
      return;
   }
#ifdef AFOUTPUT
   msg->stdOut();
#endif
   switch( msg->type())
   {
   case af::Msg::TVersionMismatch:
   {
      emit stop();
      break;
   }
   case af::Msg::TMonitorId:
   {
      if( monitor->getId() != 0 )
      {
         if( msg->int32() != monitor->getId()) connectionLost( NULL);
      }
      else
      {
         if( msg->int32() == 0)
         {
            connectionLost( NULL);
         }
         else
         {
            monitor->setId( msg->int32());
            connectionEstablished();
            afqt::QMsg * msg = new afqt::QMsg( af::Msg::TMonitorUpdateId, monitor->getId(), true);
            qthreadClientUp->setUpMsg( msg);
         }
      }
      break;
   }
   default:
      for( int t = 0; t < MTLast; t++) if( nodesList[t]->caseMessage( msg)) break;
   }
   delete msg;
}

void MonitorWindow::usersSelectionChanged()
{
   usersSelectionIds.clear();

   QList<QListWidgetItem *> usersItems = usersList->selectedItems();
   usersSelectionCount = usersItems.count();

   for( int i = 0; i < usersSelectionCount; i++) usersSelectionIds.append( ((ItemNode*)(usersItems[i]))->getId());

   jobsList->usersSelectionChanged();
}
