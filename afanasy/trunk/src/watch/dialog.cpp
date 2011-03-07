#include "dialog.h"

#include <QtGui/QBoxLayout>
#include <QtCore/QEvent>
#include <QtGui/QMenu>
#include <QtGui/QScrollArea>
#include <QtGui/QContextMenuEvent>

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "../libafqt/name_afqt.h"
#include "../libafqt/qenvironment.h"
#include "../libafqt/qmsg.h"

#include "buttonmonitor.h"
#include "buttonout.h"
#include "labelversion.h"
#include "listitems.h"
#include "listjobs.h"
#include "listusers.h"
#include "listrenders.h"
#include "listtalks.h"
#include "listmonitors.h"
#include "monitorhost.h"
#include "offlinescreen.h"
#include "watch.h"
#include "wnd.h"
#include "wndcustomizegui.h"
#include "wndtext.h"
#include "wndlist.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Dialog::Dialog():
   connected(false),
   uid(0),
   monitorType( Watch::WNONE),
   qThreadClientUpdate( this, false, af::Environment::getMonitorUpdatePeriod(), af::Environment::getMonitorConnectRetries()),
   qThreadSend( this, af::Environment::getMonitorConnectRetries()),
   qServer( this),
   listitems( NULL),
   offlinescreen( NULL),
   repaintTimer( this),
   initialized( false)
{
   if( qServer.isInitialized() == false )
   {
      AFERROR("Dialog::Dialog: Server initialization failed.\n");
      return;
   }

   for( int b = 0; b < Watch::WLAST; b++) btnMonitor[b] = NULL;

   hlayout_a = new QHBoxLayout( this);
   vlayout_a = new QVBoxLayout();
   hlayout_b = new QHBoxLayout();
   vlayout_b = new QVBoxLayout();

   hlayout_a->setMargin ( 1);
   vlayout_a->setMargin ( 1);
   hlayout_b->setMargin ( 1);
   vlayout_b->setMargin ( 1);
   hlayout_a->setSpacing( 1);
   vlayout_a->setSpacing( 1);
   hlayout_b->setSpacing( 1);
   vlayout_b->setSpacing( 1);

   btn_outlft = new ButtonOut( ButtonOut::Left,  this);
   btn_outrht = new ButtonOut( ButtonOut::Right, this);

   hlayout_a->addWidget( btn_outlft);
   hlayout_a->addLayout( vlayout_a);
   hlayout_a->addWidget( btn_outrht);
   vlayout_a->addLayout( hlayout_b);
   vlayout_a->addLayout( vlayout_b);

   hlayout_a->setAlignment( btn_outlft, Qt::AlignVCenter);
   hlayout_a->setAlignment( btn_outrht, Qt::AlignVCenter);

   infoline = new InfoLine( this);
   infoline->setMaximumHeight( ButtonMonitor::ButtonsHeight);
   vlayout_b->addWidget( infoline);

   vlayout_b->addWidget( new LabelVersion(this));

   hlayout_b->addStretch();
   btnMonitor[Watch::WJobs]    = new ButtonMonitor( Watch::WJobs,    this);
   hlayout_b->addWidget( btnMonitor[Watch::WJobs    ]);
   btnMonitor[Watch::WRenders] = new ButtonMonitor( Watch::WRenders, this);
   hlayout_b->addWidget( btnMonitor[Watch::WRenders ]);
   btnMonitor[Watch::WUsers]   = new ButtonMonitor( Watch::WUsers,   this);
   hlayout_b->addWidget( btnMonitor[Watch::WUsers   ]);
   hlayout_b->addStretch();

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
void Dialog::setDefaultWindowTitle() { setWindowTitle( QString("Watch - ") + afqt::stoq( af::Environment::getUserName()) + "@" + afqt::stoq( af::Environment::getServerName()) );}
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

   action = new QAction( "Show Offline Noise", this);
   action->setCheckable( true);
   action->setChecked( afqt::QEnvironment::showOfflineNoise.n != 0);
   connect( action, SIGNAL( triggered() ), this, SLOT( actShowOfflineNoise() ));
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
      if( afqt::QEnvironment::showOfflineNoise.n)
      {
         offlinescreen =  new OfflineScreen( listitems);
         vlayout_b->insertWidget( 0, offlinescreen);
      }
      closeList();
      ButtonMonitor::unset();
   }

   displayError("Connection lost.");
   connected = false;
   uid = 0;
   monitor->setId( 0);
   setWindowTitle( "Watch - " + afqt::stoq( af::Environment::getUserName()) + " (connecting...)");

   sendRegister();

   Watch::connectionLost();
}

void Dialog::connectionEstablished()
{
   if( offlinescreen )
   {
      delete offlinescreen;
      offlinescreen = NULL;
   }
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
   case af::Msg::TClientExitRequest:
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
         displayWarning("You does not exist.");
      }
      else
      {
         displayWarning("You have registered.");
         if( monitorType == Watch::WNONE )
         {
            ButtonMonitor::pushButton( Watch::WJobs);
         }
      }
      break;
   }
   case af::Msg::TDATA:
   {
      new WndText( "Data", msg);
      break;
   }
   case af::Msg::TStringList:
   {
      new WndText( "List", msg);
      break;
   }
   case af::Msg::TTask:
   {
      new WndText( "Task", msg);
      break;
   }
   case af::Msg::TString:
   {
      new WndText( "Message", msg);
      break;
   }
/*
   case af::Msg::TString:
   {
      std::string str;
      msg->getString( str);
      displayInfo( afqt::stoq( str));
      break;
   }
*/
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

bool Dialog::openMonitor( int type, bool open)
{
AFINFA("Dialog::openMonitor: %s[%d]\n", Watch::WndName[type].toUtf8().data(), open);

   if( open && (type == Watch::WJobs))
   {
      AFERROR("Dialog::openMonitor: Jobs monitor can't be opened is separate window.\n");
      return false;
   }

   if( !connected ) return false;
   if(( type == Watch::WJobs) && ( uid == 0 ) && ( af::Environment::VISOR() == false ))
   {
      ButtonMonitor::unset();
      displayWarning("You are not registered ( and have no jobs).");
      qThreadSend.send( new afqt::QMsg( af::Msg::TUserIdRequest, &mcuserhost, true));
      return false;
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
      return false;
   }

   if( open )
   {
      wnd->setList( newlist);
   }
   else
   {
      listitems = newlist;
      vlayout_b->insertWidget( 0, newlist);
      monitorType = type;
   }

   return true;
}

void Dialog::keyPressEvent( QKeyEvent * event)
{
   const QString key( event->text());
   if( key.isNull() || key.isEmpty() ) return;

   if( af::Environment::checkKey( key.at(0).toAscii()))
   {
      if( af::Environment::GOD())
      {
         btnMonitor[Watch::WTalks] = new ButtonMonitor( Watch::WTalks, this);
         hlayout_b->addWidget( btnMonitor[Watch::WTalks]);

         btnMonitor[Watch::WMonitors] = new ButtonMonitor( Watch::WMonitors, this);
         hlayout_b->addWidget( btnMonitor[Watch::WMonitors]);

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
      ButtonMonitor::refreshImages();
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
void Dialog::actShowOfflineNoise()        { afqt::QEnvironment::showOfflineNoise.n   = 1 - afqt::QEnvironment::showOfflineNoise.n;    }

void Dialog::actSavePreferences()
{
   if( afqt::QEnvironment::save())
      Watch::displayInfo(QString("Saved '%1'").arg( afqt::QEnvironment::getFileName()));
   else
      Watch::displayError(QString("Failed to save to'%1'").arg( afqt::QEnvironment::getFileName()));
}
