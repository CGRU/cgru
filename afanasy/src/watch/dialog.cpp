#include "dialog.h"

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "../libafqt/name_afqt.h"
#include "../libafqt/qenvironment.h"

#include "actionid.h"
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
#include "wndcustomizesounds.h"
#include "wndtask.h"
#include "wndtext.h"
#include "wndlist.h"

#include <QtCore/QEvent>
#include <QtGui/QBoxLayout>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QMenu>
#include <QtGui/QPainter>
#include <QtGui/QScrollArea>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

int Dialog::ms_size_border_top   = 40;
int Dialog::ms_size_border_bot   = 25;
int Dialog::ms_size_border_left  = 65;
int Dialog::ms_size_border_right = 75;

Dialog::Dialog():
    m_connected(false),
    m_uid(0),
    m_monitorType( Watch::WNONE),
    m_qThreadClientUpdate( this, false, af::Environment::getMonitorUpdatePeriod(), af::Environment::getMonitorConnectRetries()),
    m_qThreadSend( this, af::Environment::getMonitorConnectRetries()),
    m_qServer( this),
    m_listitems( NULL),
    m_offlinescreen( NULL),
    m_repaintTimer( this),
    m_initialized( false)
{
    if( m_qServer.isInitialized() == false )
    {
        AFERROR("Dialog::Dialog: Server initialization failed.")
        return;
    }

    for( int b = 0; b < Watch::WLAST; b++) m_btnMonitor[b] = NULL;

    m_hlayout_a = new QHBoxLayout( this);
    m_vlayout_a = new QVBoxLayout();
    m_hlayout_b = new QHBoxLayout();
    m_vlayout_b = new QVBoxLayout();

    m_hlayout_a->setMargin ( 0);
    m_vlayout_a->setMargin ( 0);
    m_hlayout_b->setMargin ( 0);
    m_vlayout_b->setMargin ( 0);
    m_hlayout_a->setSpacing( 0);
    m_vlayout_a->setSpacing( 0);
    m_hlayout_b->setSpacing( 0);
    m_vlayout_b->setSpacing( 0);

    m_btn_out_left = new ButtonOut( ButtonOut::Left,  this);
    m_btn_out_right = new ButtonOut( ButtonOut::Right, this);

    m_hlayout_a->addWidget( m_btn_out_left);
    m_hlayout_a->addLayout( m_vlayout_a);
    m_hlayout_a->addWidget( m_btn_out_right);
    m_vlayout_a->addLayout( m_hlayout_b);
    m_vlayout_a->addLayout( m_vlayout_b);

    m_hlayout_a->setAlignment( m_btn_out_left, Qt::AlignVCenter);
    m_hlayout_a->setAlignment( m_btn_out_right, Qt::AlignVCenter);

    m_infoline = new InfoLine( this);
    m_infoline->setMaximumHeight( ButtonMonitor::ButtonsHeight);
    m_vlayout_b->addWidget( m_infoline);

    m_labelversion = new LabelVersion(this);
    m_vlayout_b->addWidget( m_labelversion);

    m_topleft  = new QWidget( this);
    m_topright = new QWidget( this);
    m_hlayout_b->addWidget( m_topleft);
    m_btnMonitor[Watch::WJobs]    = new ButtonMonitor( Watch::WJobs,    this);
    m_hlayout_b->addWidget( m_btnMonitor[Watch::WJobs    ]);
    m_btnMonitor[Watch::WRenders] = new ButtonMonitor( Watch::WRenders, this);
    m_hlayout_b->addWidget( m_btnMonitor[Watch::WRenders ]);
    m_btnMonitor[Watch::WUsers]   = new ButtonMonitor( Watch::WUsers,   this);
    m_hlayout_b->addWidget( m_btnMonitor[Watch::WUsers   ]);
    m_hlayout_b->addWidget( m_topright);

    connect( &m_qServer,               SIGNAL( newMsg( af::Msg*)), this, SLOT( newMessage( af::Msg*)));
    connect( &m_qThreadSend,           SIGNAL( newMsg( af::Msg*)), this, SLOT( newMessage( af::Msg*)));
    connect( &m_qThreadClientUpdate,   SIGNAL( newMsg( af::Msg*)), this, SLOT( newMessage( af::Msg*)));
    connect( &m_qThreadClientUpdate,   SIGNAL( connectionLost()),  this, SLOT( connectionLost()));
    connect( &m_qThreadSend,           SIGNAL( connectionLost()),  this, SLOT( connectionLost()));

    m_monitor = new MonitorHost();

    connectionLost();

    connect( &m_repaintTimer, SIGNAL( timeout()), this, SLOT( repaintWatch()));

    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground( true);
    QRect rect;
    if( afqt::QEnvironment::getRect( "Main", rect)) setGeometry( rect);

    displayInfo("Ready.");
    m_initialized = true;

    reloadImages();
    Watch::refreshGui();
}

void Dialog::closeEvent( QCloseEvent * event) { afqt::QEnvironment::setRect( "Main", geometry());}

Dialog::~Dialog()
{
    AFINFO("Dialog::~Dialog:")
    Watch::destroy();
	if( m_connected )
		m_qThreadSend.send( new af::Msg( af::Msg::TMonitorDeregister, m_monitor->getId()));
    delete m_monitor;
}

void Dialog::repaintStart( int mseconds) { m_repaintTimer.start( mseconds);}
void Dialog::repaintFinish()             { m_repaintTimer.stop();}
void Dialog::setDefaultWindowTitle() { setWindowTitle( QString("Watch - ") + afqt::stoq( af::Environment::getUserName()) + "@" + afqt::stoq( af::Environment::getServerName()) );}
void Dialog::sendRegister(){ m_qThreadClientUpdate.setUpMsg( new af::Msg( af::Msg::TMonitorRegister, m_monitor, true));}
void Dialog::sendMsg( af::Msg * msg)
{
#ifdef AFOUTPUT
printf(" <<< Dialog::sendMsg: ");msg->stdOut();
#endif
    m_qThreadSend.send( msg);
}

void Dialog::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    QAction *action;
    QMenu * submenu;

    submenu = new QMenu( "Choose Theme", this);
    QStringList themes = afqt::QEnvironment::getThemes();
    for( int i = 0; i < themes.size(); i++)
    {
        ActionString * action_str = new ActionString( themes[i], themes[i], this);
        action_str->setCheckable( true);
        action_str->setChecked( afqt::QEnvironment::theme.str == themes[i]);
        connect( action_str, SIGNAL( triggeredString(QString)), this, SLOT( actGuiTheme(QString)));
        submenu->addAction( action_str);
    }
    menu.addMenu( submenu);

    action = new QAction( "Customize GUI...", this);
    connect( action, SIGNAL( triggered() ), this, SLOT( actColors() ));
    menu.addAction( action);

    action = new QAction( "Sound Events...", this);
    connect( action, SIGNAL( triggered() ), this, SLOT( actSounds() ));
    menu.addAction( action);

    menu.addSeparator();

    submenu = new QMenu( "Preferences", this);

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

void Dialog::connectionLost()
{
    if( m_monitorType == Watch::WJobs )
    {
        if( afqt::QEnvironment::showOfflineNoise.n)
        {
            m_offlinescreen =  new OfflineScreen( m_listitems);
            m_vlayout_b->insertWidget( 0, m_offlinescreen);
        }
        closeList();
        ButtonMonitor::unset();
    }

    displayError("Connection lost.");
    m_connected = false;
    m_uid = 0;
    m_monitor->setId( 0);
    setWindowTitle( "Watch - " + afqt::stoq( af::Environment::getUserName()) + " (connecting...)");

    sendRegister();

    Watch::connectionLost();
}

void Dialog::connectionEstablished()
{
    if( m_offlinescreen )
    {
        delete m_offlinescreen;
        m_offlinescreen = NULL;
    }
    displayInfo("Connection established.");
    m_connected = true;
    setDefaultWindowTitle();
    m_qThreadSend.send( new af::Msg( af::Msg::TUserIdRequest, &m_mcuserhost, true));

    Watch::connectionEstablished();
}

void Dialog::newMessage( af::Msg *msg)
{
    if( msg == NULL)
    {
        AFERROR("Dialog::caseMessage: msg == NULL")
        return;
    }
    #ifdef AFOUTPUT
    printf(" >>> Dialog::newMessage: ");msg->stdOut();
    #endif
    switch( msg->type())
    {
    case af::Msg::TClientExitRequest:
    case af::Msg::TVersionMismatch:
    case af::Msg::TMagicMismatch:
    {
        emit stop();
        break;
    }
    case af::Msg::TMonitorId:
    {
        if( m_monitor->getId() != 0 )
        {
            if( msg->int32() != m_monitor->getId()) connectionLost();
        }
        else
        {
            if( msg->int32() == 0)
            {
                connectionLost();
            }
            else
            {
                m_monitor->setId( msg->int32());
                connectionEstablished();
                af::Msg * msg = new af::Msg( af::Msg::TMonitorUpdateId, m_monitor->getId(), true);
                m_qThreadClientUpdate.setUpMsg( msg);
            }
        }
        break;
    }
    case af::Msg::TUserId:
    {
        m_uid = msg->int32();
        if( m_uid == 0 )
        {
            if( m_monitorType == Watch::WJobs )
            {
                ButtonMonitor::unset();
                closeList();
            }
            displayWarning("You do not exist.");
        }
        else
        {
            displayInfo("You have registered.");
            if( m_monitorType == Watch::WNONE )
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
        new WndTask( "Task", msg);
        break;
    }
    case af::Msg::TString:
    {
        std::string str = msg->getString();
        if( LabelVersion::getStringStatus( str) != LabelVersion::SS_None )
            m_labelversion->showMessage( str);
        else
            new WndText( "Message", msg);
        break;
    }
    default:
        Watch::caseMessage( msg);
    }
    AFINFO("Dialog::newMessage: Reaction finished deleting message.")
    delete msg;
    AFINFO("Dialog::newMessage: Message successfully deleted.")
}

void Dialog::closeList()
{
    if( m_listitems != NULL) m_listitems->close();
    m_listitems = NULL;
    m_monitorType = Watch::WNONE;
}

bool Dialog::openMonitor( int type, bool open)
{
   AFINFA("Dialog::openMonitor: %s[%d]", Watch::WndName[type].toUtf8().data(), open)

   if( open && (type == Watch::WJobs))
   {
      AFERROR("Dialog::openMonitor: Jobs monitor can't be opened is separate window.")
      return false;
   }

   if( !m_connected ) return false;
   if(( type == Watch::WJobs) && ( m_uid == 0 ) && ( af::Environment::VISOR() == false ))
   {
      ButtonMonitor::unset();
      displayWarning("You are not registered ( and have no jobs).");
      m_qThreadSend.send( new af::Msg( af::Msg::TUserIdRequest, &m_mcuserhost, true));
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
      AFERRAR("Dialog::changeMonitor: unknown type = %d", type)
      if( false == open ) m_monitorType = Watch::WNONE;
      return false;
   }

   if( open )
   {
      wnd->setList( newlist);
   }
   else
   {
      m_listitems = newlist;
      m_vlayout_b->insertWidget( 0, newlist);
      m_monitorType = type;
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
         m_btnMonitor[Watch::WTalks] = new ButtonMonitor( Watch::WTalks, this);
         m_hlayout_b->addWidget( m_btnMonitor[Watch::WTalks]);

         m_btnMonitor[Watch::WMonitors] = new ButtonMonitor( Watch::WMonitors, this);
         m_hlayout_b->addWidget( m_btnMonitor[Watch::WMonitors]);

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
         if( m_btnMonitor[Watch::WTalks])
         {
            delete m_btnMonitor[Watch::WTalks];
            m_btnMonitor[Watch::WTalks] = NULL;
         }
         if( m_btnMonitor[Watch::WMonitors])
         {
            delete m_btnMonitor[Watch::WMonitors];
            m_btnMonitor[Watch::WMonitors] = NULL;
         }

         setBackgroundRole( QPalette::NoRole );
         displayWarning("SUPER USER MODE OFF");
      }
      int opened_type = m_monitorType ;
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

void Dialog::actSounds()
{
    new WndCustomizeSounds();
    Watch::displayInfo("Opening 'Sounds' Window");
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

void Dialog::actGuiTheme( QString theme)
{
    if( afqt::QEnvironment::loadTheme( theme))
    {
        Watch::refreshGui();
        //repaintWatch();
        Watch::displayInfo(QString("Theme '%1' loaded").arg( theme));
    }
    else
        Watch::displayError(QString("Failed to load '%1' theme").arg( theme));
}

void Dialog::reloadImages()
{
    Watch::loadImage( m_img_top,      afqt::QEnvironment::image_border_top.str      );
    Watch::loadImage( m_img_topleft,  afqt::QEnvironment::image_border_topleft.str  );
    Watch::loadImage( m_img_topright, afqt::QEnvironment::image_border_topright.str );
    Watch::loadImage( m_img_bot,      afqt::QEnvironment::image_border_bot.str      );
    Watch::loadImage( m_img_botleft,  afqt::QEnvironment::image_border_botleft.str  );
    Watch::loadImage( m_img_botright, afqt::QEnvironment::image_border_botright.str );

    if( m_img_top.isNull() )
    {
        m_topleft->setFixedHeight( ms_size_border_top);
        m_topright->setFixedHeight( ms_size_border_top);
    }
    else
    {
        m_topleft->setFixedHeight( m_img_top.height());
        m_topright->setFixedHeight( m_img_top.height());
    }

    if( m_img_bot.isNull())
        m_labelversion->setFixedHeight( ms_size_border_bot);
    else
        m_labelversion->setFixedHeight( m_img_bot.height());

    m_btn_out_left->reloadImages();
    m_btn_out_right->reloadImages();
}

void Dialog::paintEvent( QPaintEvent * event )
{
//return;
//printf("Dialog::paintEvent:\n");
    QPainter p( this);

    QRect r = rect();

    if( false == m_img_top.isNull())
        p.drawPixmap( r.width()/2 - m_img_top.width()/2, 0, m_img_top);

    if( false == m_img_topleft.isNull())
        p.drawPixmap( 0, 0, m_img_topleft);

    if( false == m_img_topright.isNull())
        p.drawPixmap( r.width() - m_img_topright.width(), 0, m_img_topright);

    if( false == m_img_bot.isNull())
        p.drawPixmap( r.width()/2 - m_img_bot.width()/2, r.height() - m_img_bot.height(), m_img_bot);

    if( false == m_img_botleft.isNull())
        p.drawPixmap( 0, r.height() - m_img_botleft.height(), m_img_botleft);

    if( false == m_img_botright.isNull())
        p.drawPixmap( r.width() - m_img_botright.width(), r.height() - m_img_botright.height(), m_img_botright);

    //QWidget::paintEvent( event );
}

void Dialog::repaintWatch()
{
    if ( m_listitems ) m_listitems->repaintItems();
    Watch::repaint();
}
