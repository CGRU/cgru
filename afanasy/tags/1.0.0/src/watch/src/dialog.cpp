#include "dialog.h"

#include <QtGui/QBoxLayout>
#include <QtCore/QEvent>
#include <QtGui/QMenu>
#include <QtGui/QScrollArea>
#include <QtGui/QContextMenuEvent>

#include <afanasy.h>
#include <environment.h>
#include <msgclasses/mcgeneral.h>

#include <name_afqt.h>
#include <qenvironment.h>
#include <qmsg.h>

#include "buttonmonitor.h"
#include "buttonout.h"
#include "listitems.h"
#include "listjobs.h"
#include "listusers.h"
#include "listrenders.h"
#include "listtalks.h"
#include "listmonitors.h"
#include "monitorhost.h"
#include "watch.h"
#include "wnd.h"
#include "wndcustomizegui.h"
#include "wndtext.h"
#include "wndlist.h"

//#define AFOUTPUT
//#undef AFOUTPUT
#include <macrooutput.h>

Dialog::Dialog():
   connected(false),
   uid(0),
   monitorType( Watch::WNONE),
   qThreadClientUpdate( this, false, af::Environment::getMonitorUpdatePeriod(), af::Environment::getMonitorConnectRetries()),
   qThreadSend( this, af::Environment::getMonitorConnectRetries()),
   qServer( this),
   listitems( NULL),
   repaintTimer( this),
   initialized( false)
{
   if( qServer.isInitialized() == false )
   {
      AFERROR("Dialog::Dialog: Server initialization failed.\n");
      return;
   }

   for( int b = 0; b < Watch::WLAST; b++) btnMonitor[b] = NULL;

   hlayoutB = new QHBoxLayout( this);
   vlayoutB = new QVBoxLayout();
   hlayout = new QHBoxLayout();
   vlayout = new QVBoxLayout();

   hlayoutB->setMargin ( 1);
   vlayoutB->setMargin ( 1);
   hlayout ->setMargin ( 1);
   vlayout ->setMargin ( 1);
   hlayoutB->setSpacing( 1);
   vlayoutB->setSpacing( 1);
   hlayout ->setSpacing( 1);
   vlayout ->setSpacing( 1);

   btn_outlft = new ButtonOut( ButtonOut::Left,  this);
   btn_outrht = new ButtonOut( ButtonOut::Right, this);

   hlayoutB->addWidget( btn_outlft);
   hlayoutB->addLayout( vlayoutB);
   hlayoutB->addWidget( btn_outrht);
   vlayoutB->addLayout( hlayout);
   vlayoutB->addLayout( vlayout);

   hlayoutB->setAlignment( btn_outlft, Qt::AlignVCenter);
   hlayoutB->setAlignment( btn_outrht, Qt::AlignVCenter);

   infoline = new InfoLine( this);
   infoline->setMaximumHeight( ButtonMonitor::ButtonsHeight);
   vlayout->addWidget( infoline);

   btnMonitor[Watch::WJobs]    = new ButtonMonitor( Watch::WJobs,    this);
   hlayout->addWidget( btnMonitor[Watch::WJobs    ]);
   btnMonitor[Watch::WRenders] = new ButtonMonitor( Watch::WRenders, this);
   hlayout->addWidget( btnMonitor[Watch::WRenders ]);
   btnMonitor[Watch::WUsers]   = new ButtonMonitor( Watch::WUsers,   this);
   hlayout->addWidget( btnMonitor[Watch::WUsers   ]);

   setFocusPolicy(Qt::StrongFocus);

   connect( &qServer,               SIGNAL( newMsg( af::Msg*)), this, SLOT( newMessage( af::Msg*)));
   connect( &qThreadSend,           SIGNAL( newMsg( af::Msg*)), this, SLOT( newMessage( af::Msg*)));
   connect( &qThreadClientUpdate,   SIGNAL( newMsg( af::Msg*)), this, SLOT( newMessage( af::Msg*)));
   connect( &qThreadClientUpdate,   SIGNAL( connectionLost( af::Address*)), this, SLOT( connectionLost( af::Address*)));
   connect( &qThreadSend,           SIGNAL( connectionLost( af::Address*)), this, SLOT( connectionLost( af::Address*)));

   setAutoFillBackground( true);

   monitor = new MonitorHost();

   connectionLost( NULL);

   connect( &repaintTimer, SIGNAL( timeout()), this, SLOT( repaintWatch()));

   displayInfo("Ready.");
   initialized = true;

   QRect rect;
   if( afqt::QEnvironment::getRect( "Main", rect)) setGeometry( rect);
}

void Dialog::closeEvent( QCloseEvent * event) { afqt::QEnvironment::setRect( "Main", geometry());}

Dialog::~Dialog()
{
AFINFO("Dialog::~Dialog:\n");
   Watch::destroy();
   qThreadSend.send( new afqt::QMsg( af::Msg::TMonitorDeregister, monitor->getId()));
   delete monitor;
}

void Dialog::repaintStart( int mseconds) { repaintTimer.start( mseconds);                             }
void Dialog::repaintFinish()             { repaintTimer.stop();                                       }
void Dialog::repaintWatch()              { Watch::repaint(); if(listitems) listitems->repaintItems(); }
void Dialog::setDefaultWindowTitle() { setWindowTitle( "Watch - " + af::Environment::getUserName() + "@" + af::Environment::getServerName());}
void Dialog::sendRegister(){ qThreadClientUpdate.setUpMsg( new afqt::QMsg( af::Msg::TMonitorRegister, monitor, true));}
void Dialog::sendMsg( afqt::QMsg * msg)
{
#ifdef AFOUTPUT
printf(" <<< Dialog::sendMsg: ");msg->stdOut();
#endif
   qThreadSend.send( msg);
}

void Dialog::contextMenuEvent(QContextMenuEvent *event)
{
   QMenu menu(this);
   QAction *action;

   action = new QAction( "Customize GUI...", this);
   connect( action, SIGNAL( triggered() ), this, SLOT( actColors() ));
   menu.addAction( action);

   menu.addSeparator();

   QMenu * submenu = new QMenu( "Preferences", this);

   action = new QAction( "Save Prefs on Exit", this);
   action->setCheckable( true);
   action->setChecked( afqt::QEnvironment::savePrefsOnExit.n != 0);
   connect( action, SIGNAL( triggered() ), this, SLOT( actSavePreferencesOnExit() ));
   submenu->addAction( action);

   action = new QAction( "Save GUI", this);
   action->setCheckable( true);
   action->setChecked( afqt::QEnvironment::saveGUIOnExit.n != 0);
   connect( action, SIGNAL( triggered() ), this, SLOT( actSaveGUIOnExit() ));
   submenu->addAction( action);

   action = new QAction( "Save Windows Geometry", this);
   action->setCheckable( true);
   action->setChecked( afqt::QEnvironment::saveWndRectsOnExit.n != 0);
   connect( action, SIGNAL( triggered() ), this, SLOT( actSaveWndRectsOnExit() ));
   submenu->addAction( action);

   menu.addMenu( submenu);

   action = new QAction( "Save Preferences", this);
   connect( action, SIGNAL( triggered() ), this, SLOT( actSavePreferences() ));
   menu.addAction( action);

   menu.exec( event->globalPos());
}

void Dialog::connectionLost( af::Address* address)
{
   if( monitorType == Watch::WJobs )
   {
      closeList();
      ButtonMonitor::unset();
   }

   displayError("Connection lost.");
   connected = false;
   uid = 0;
   monitor->setId( 0);
   setWindowTitle( "Watch - " + af::Environment::getUserName() + " (connecting...)");

   sendRegister();

   Watch::connectionLost();
}

void Dialog::connectionEstablished()
{
   displayInfo("Connection established.");
   connected = true;
   setDefaultWindowTitle();
   qThreadSend.send( new afqt::QMsg( af::Msg::TUserIdRequest, &mcuserhost, true));

   Watch::connectionEstablished();
}

void Dialog::newMessage( af::Msg *msg)
{
   if( msg == NULL)
   {
      AFERROR("Dialog::caseMessage: msg == NULL\n");
      return;
   }
#ifdef AFOUTPUT
printf(" >>> Dialog::newMessage: ");msg->stdOut();
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
            qThreadClientUpdate.setUpMsg( msg);
         }
      }
      break;
   }
   case af::Msg::TUserId:
   {
      uid = msg->int32();
      if( uid == 0 )
      {
         if( monitorType == Watch::WJobs )
         {
            ButtonMonitor::unset();
            closeList();
         }
         displayWarning("You are not exists.");
      }
      else
      {
         displayWarning("You have registered.");
         if( monitorType == Watch::WNONE )
         {
            btnMonitor[Watch::WJobs]->pressed_SLOT();
         }
      }
      break;
   }
   case af::Msg::TDATA:
   {
      new WndText( "Data", msg);
      break;
   }
   case af::Msg::TQStringList:
   {
      new WndText( "List", msg);
      break;
   }
   case af::Msg::TTask:
   {
      new WndText( "Task", msg);
      break;
   }
   case af::Msg::TQString:
   {
      QString str;
      msg->getString( str);
      displayInfo( str);
      break;
   }
   default:
      Watch::caseMessage( msg);
   }
AFINFO("Dialog::newMessage: Reaction finished deleting message.\n");
   delete msg;
AFINFO("Dialog::newMessage: Message successfully deleted.\n");
}

void Dialog::closeList()
{
   if( listitems != NULL) listitems->close();
   listitems = NULL;
   monitorType = Watch::WNONE;
}

void Dialog::openMonitor( int type, bool open)
{
AFINFA("Dialog::openMonitor: %s[%d]\n", Watch::WndName[type].toUtf8().data(), open);
   if( !connected ) return;
   if(( type == Watch::WJobs) && ( uid == 0 ) && ( af::Environment::VISOR() == false ))
   {
      ButtonMonitor::unset();
      displayWarning("You are not registered ( and have no jobs).");
      qThreadSend.send( new afqt::QMsg( af::Msg::TUserIdRequest, &mcuserhost, true));
      return;
   }

   QWidget * parent = this;
   ListItems * newlist = NULL;
   WndList * wnd = NULL;
   if( open )
   {
      wnd = new WndList( Watch::WndName[type], type);
      parent = wnd;
   }
   else
   {
      closeList();
   }

   switch( type)
   {
   case Watch::WJobs:
   {
      newlist = new ListJobs( parent);
      displayInfo("Your jobs list.");
      break;
   }
   case Watch::WUsers:
   {
      newlist = new ListUsers( parent);
      displayInfo("Users list.");
     break;
   }
   case Watch::WRenders:
   {
      newlist = new ListRenders( parent);
      displayInfo("Render hosts list.");
      break;
   }
   case Watch::WTalks:
   {
      newlist = new ListTalks( parent);
      displayInfo("Talks chat dialogs.");
      break;
   }
   case Watch::WMonitors:
   {
      newlist = new ListMonitors( parent);
      displayInfo("Connected monitors.");
      break;
   }
   default:
      AFERRAR("Dialog::changeMonitor: unknown type = %d.\n", type);
      if( false == open ) monitorType = Watch::WNONE;
      return;
   }

   if( open )
   {
      wnd->setList( newlist);
   }
   else
   {
      listitems = newlist;
      vlayout->insertWidget( 0, newlist);
      monitorType = type;
   }
}

void Dialog::keyPressEvent( QKeyEvent * event)
{
   const QString key( event->text());
   if( key.isNull() || key.isEmpty() ) return;

   if( af::Environment::checkKey( key.at(0).toAscii()))
   {
      if( af::Environment::GOD())
      {
         btnMonitor[Watch::WTalks] = new ButtonMonitor( Watch::WTalks, this, 40, 20);
         hlayout->addWidget( btnMonitor[Watch::WTalks]);

         btnMonitor[Watch::WMonitors] = new ButtonMonitor( Watch::WMonitors, this, 40, 20);
         hlayout->addWidget( btnMonitor[Watch::WMonitors]);

         setBackgroundRole( QPalette::LinkVisited );
         displayWarning("GOD MODE");
      }
      else if( af::Environment::VISOR())
      {
         setBackgroundRole( QPalette::Link );
         displayWarning("VISOR MODE");
      }
      else
      {
         if( btnMonitor[Watch::WTalks])
         {
            delete btnMonitor[Watch::WTalks];
            btnMonitor[Watch::WTalks] = NULL;
         }
         if( btnMonitor[Watch::WMonitors])
         {
            delete btnMonitor[Watch::WMonitors];
            btnMonitor[Watch::WMonitors] = NULL;
         }

         setBackgroundRole( QPalette::NoRole );
         displayWarning("SUPER USER MODE OFF");
      }
      int opened_type = monitorType ;
      closeList();
//      ButtonMonitor::unset();
      setDefaultWindowTitle();
      if( opened_type != Watch::WNONE) openMonitor( opened_type, false);
   }
}

void Dialog::actColors()
{
   new WndCustomizeGUI();
   Watch::displayInfo("Opening 'GUI' Window");
}

void Dialog::actSavePreferencesOnExit()   { afqt::QEnvironment::savePrefsOnExit.n    = 1 - afqt::QEnvironment::savePrefsOnExit.n;     }
void Dialog::actSaveGUIOnExit()           { afqt::QEnvironment::saveGUIOnExit.n      = 1 - afqt::QEnvironment::saveGUIOnExit.n;       }
void Dialog::actSaveWndRectsOnExit()      { afqt::QEnvironment::saveWndRectsOnExit.n = 1 - afqt::QEnvironment::saveWndRectsOnExit.n;  }

void Dialog::actSavePreferences()
{
   if( afqt::QEnvironment::save())
      Watch::displayInfo(QString("Saved '%1'").arg( afqt::QEnvironment::getFileName()));
   else
      Watch::displayError(QString("Failed to save to'%1'").arg( afqt::QEnvironment::getFileName()));
}
