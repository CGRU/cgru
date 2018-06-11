#include "dialog.h"

#include "../include/afanasy.h"
#include "../include/afgui.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mctaskup.h"

#include "../libafqt/name_afqt.h"
#include "../libafqt/qenvironment.h"

#include "actionid.h"
#include "buttonmonitor.h"
#include "buttonout.h"
#include "listitems.h"
#include "listjobs.h"
#include "listusers.h"
#include "listrenders.h"
#include "listmonitors.h"
#include "monitorhost.h"
#include "offlinescreen.h"
#include "watch.h"
#include "wnd.h"
#include "wndcustomizegui.h"
#include "wndnotifications.h"
#include "wndtask.h"
#include "wndtext.h"
#include "wndlist.h"

#include <QtCore/QEvent>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QPainter>
#include <QBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QScrollArea>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

int Dialog::ms_size_border_top   = 40;
int Dialog::ms_size_border_bot   = 25;
int Dialog::ms_size_border_left  = 65;
int Dialog::ms_size_border_right = 75;

Dialog::Dialog():
    m_connected(false),
    m_monitorType( Watch::WNONE),
	m_qafclient( this, af::Environment::getWatchConnectRetries()),
    m_listitems( NULL),
    m_offlinescreen( NULL),
    m_repaintTimer( this),
    m_initialized( false)
{
    for( int b = 0; b < Watch::WLAST; b++) m_btnMonitor[b] = NULL;

	QWidget * cw = new QWidget( this);
	setCentralWidget( cw);

    m_hlayout_a = new QHBoxLayout( cw);
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
    m_vlayout_b->setSpacing( 3);

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

//    m_topleft  = new QWidget( this);
	m_topleft  = new QLabel("", this);
    m_topright = new QWidget( this);
    m_hlayout_b->addWidget( m_topleft);
    m_btnMonitor[Watch::WJobs]    = new ButtonMonitor( Watch::WJobs,    this);
    m_hlayout_b->addWidget( m_btnMonitor[Watch::WJobs    ]);
    m_btnMonitor[Watch::WRenders] = new ButtonMonitor( Watch::WRenders, this);
    m_hlayout_b->addWidget( m_btnMonitor[Watch::WRenders ]);
    m_btnMonitor[Watch::WUsers]   = new ButtonMonitor( Watch::WUsers,   this);
    m_hlayout_b->addWidget( m_btnMonitor[Watch::WUsers   ]);
    m_hlayout_b->addWidget( m_topright);
	
	connect( &m_qafclient, SIGNAL( sig_newMsg( af::Msg*)), this, SLOT( newMessage( af::Msg*)));
	connect( &m_qafclient, SIGNAL( sig_connectionLost()),  this, SLOT( connectionLost()));
	connect( &m_qafclient, SIGNAL( sig_finished()),        this, SLOT( close()));

    connectionLost();

	createMenus();
	if( Watch::isSith())
		menuBar()->hide();

    connect( &m_repaintTimer, SIGNAL( timeout()), this, SLOT( repaintWatch()));

//    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground( true);
    QRect rect;
    if( afqt::QEnvironment::getRect( "Main", rect)) setGeometry( rect);

    displayInfo("Ready.");
    m_initialized = true;

    reloadImages();
    Watch::refreshGui();
}

void Dialog::closeEvent( QCloseEvent * event)
{
	static bool s_closing = false;
	if( s_closing ) return;
	s_closing = true;

	// Ignore event to not exit Qt event loop,
	// as it needed for signal -> slot events processing.
	event->ignore();

	if( m_connected )
	{
		AF_LOG << "Sending deregister request.";
		m_qafclient.sendMsg( new af::Msg( af::Msg::TMonitorDeregister, MonitorHost::id()));
		m_qafclient.setClosing();
	}

	afqt::QEnvironment::setRect( "Main", geometry());
}

Dialog::~Dialog()
{
	AF_DEBUG;
}

void Dialog::repaintStart( int mseconds) { m_repaintTimer.start( mseconds);}
void Dialog::repaintFinish()             { m_repaintTimer.stop();}
void Dialog::setDefaultWindowTitle() { setWindowTitle( QString("Watch - ") + afqt::stoq( af::Environment::getUserName()) + "@" + afqt::stoq( af::Environment::getServerName()) );}
void Dialog::sendRegister(){ m_qafclient.setUpMsg( MonitorHost::genRegisterMsg(), af::Environment::getWatchGetEventsSec());}
void Dialog::sendMsg( af::Msg * msg)
{
#ifdef AFOUTPUT
printf(" <<< Dialog::sendMsg: ");msg->v_stdOut();
#endif
    m_qafclient.sendMsg( msg);
}

void Dialog::createMenus()
{
	m_contextMenu = new QMenu( this);

	m_levelMenu = new QMenu("UI &Level", this);
	menuBar()->addMenu( m_levelMenu);
	m_contextMenu->addMenu( m_levelMenu);
	connect( m_levelMenu, SIGNAL( aboutToShow()), this, SLOT( showMenuLevel()));

	m_themeMenu = new QMenu("Color &Theme", this);
	menuBar()->addMenu( m_themeMenu);
	m_contextMenu->addMenu( m_themeMenu);
	connect( m_themeMenu, SIGNAL( aboutToShow()), this, SLOT( showMenuTheme()));

	m_contextMenu->addSeparator();

    QAction *action;
	QMenu * editMenu = menuBar()->addMenu("&Edit");
    action = new QAction( "Customize GUI...", editMenu);
    connect( action, SIGNAL( triggered() ), this, SLOT( actColors() ));
    editMenu->addAction( action);
	m_contextMenu->addAction( action);
    action = new QAction( "Notifications...", editMenu);
    connect( action, SIGNAL( triggered() ), this, SLOT( actNotifications() ));
    editMenu->addAction( action);
	m_contextMenu->addAction( action);
    editMenu->addSeparator();

	m_prefsMenu = new QMenu("&Preferences", this);
	menuBar()->addMenu( m_prefsMenu);
	m_contextMenu->addMenu( m_prefsMenu);
	connect( m_prefsMenu, SIGNAL( aboutToShow()), this, SLOT( showMenuPrefs()));

	m_contextMenu->addSeparator();

    action = new QAction( "Save Preferences", editMenu);
    connect( action, SIGNAL( triggered() ), this, SLOT( actSavePreferences() ));
    editMenu->addAction( action);
	m_contextMenu->addAction( action);

	m_helpMenu = new QMenu("&Help", this);
	menuBar()->addMenu( m_helpMenu);
	m_contextMenu->addMenu( m_helpMenu);
	connect( m_helpMenu, SIGNAL( aboutToShow()), this, SLOT( showMenuHelp()));
}

void Dialog::showMenuLevel()
{
	m_levelMenu->clear();

	const char *jedi_names[] = {"Padawan","Jedi","Sith"};
    for( int i = 0; i < AFGUI::SITH+1; i++)
    {
        ActionId * action_id = new ActionId( i, jedi_names[i], m_levelMenu);
        action_id->setCheckable( true);
        action_id->setChecked( afqt::QEnvironment::level.n == i);
        connect( action_id, SIGNAL( triggeredId(int)), this, SLOT( actGuiLevel(int)));
        m_levelMenu->addAction( action_id);
    }

}

void Dialog::showMenuTheme()
{
	m_themeMenu->clear();

    QStringList themes = afqt::QEnvironment::getThemes();
    for( int i = 0; i < themes.size(); i++)
    {
        ActionString * action_str = new ActionString( themes[i], themes[i], m_themeMenu);
        action_str->setCheckable( true);
        action_str->setChecked( afqt::QEnvironment::theme.str == themes[i]);
        connect( action_str, SIGNAL( triggeredString(QString)), this, SLOT( actGuiTheme(QString)));
        m_themeMenu->addAction( action_str);
    }
}

void Dialog::showMenuPrefs()
{
	m_prefsMenu->clear();
	QAction * action;

    action = new QAction( "Save Prefs on Exit", m_prefsMenu);
    action->setCheckable( true);
    action->setChecked( afqt::QEnvironment::savePrefsOnExit.n != 0);
    connect( action, SIGNAL( triggered() ), this, SLOT( actSavePreferencesOnExit() ));
    m_prefsMenu->addAction( action);

	m_prefsMenu->addSeparator();

    action = new QAction( "Save GUI", m_prefsMenu);
    action->setCheckable( true);
    action->setChecked( afqt::QEnvironment::saveGUIOnExit.n != 0);
    connect( action, SIGNAL( triggered() ), this, SLOT( actSaveGUIOnExit() ));
    m_prefsMenu->addAction( action);

    action = new QAction( "Save Hotkeys", m_prefsMenu);
    action->setCheckable( true);
    action->setChecked( afqt::QEnvironment::saveHotkeysOnExit.n != 0);
    connect( action, SIGNAL( triggered() ), this, SLOT( actSaveHotkeysOnExit() ));
    m_prefsMenu->addAction( action);

    action = new QAction( "Save Windows Geometry", m_prefsMenu);
    action->setCheckable( true);
    action->setChecked( afqt::QEnvironment::saveWndRectsOnExit.n != 0);
    connect( action, SIGNAL( triggered() ), this, SLOT( actSaveWndRectsOnExit() ));
    m_prefsMenu->addAction( action);

	m_prefsMenu->addSeparator();

    action = new QAction( "Show Offline Noise", m_prefsMenu);
    action->setCheckable( true);
    action->setChecked( afqt::QEnvironment::showOfflineNoise.n != 0);
    connect( action, SIGNAL( triggered() ), this, SLOT( actShowOfflineNoise() ));
    m_prefsMenu->addAction( action);
}

void Dialog::showMenuHelp()
{
	m_helpMenu->clear();
	QAction * action;

    action = new QAction("Documentation...", m_helpMenu);
    connect( action, SIGNAL( triggered() ), this, SLOT( actShowDocs() ));
    m_helpMenu->addAction( action);

    action = new QAction("Forum...", m_helpMenu);
    connect( action, SIGNAL( triggered() ), this, SLOT( actShowForum() ));
    m_helpMenu->addAction( action);
}

void Dialog::contextMenuEvent(QContextMenuEvent *event)
{
	if( Watch::isPadawan())
		return;

	m_contextMenu->exec( event->globalPos());
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
    setWindowTitle( "Watch - " + afqt::stoq( af::Environment::getUserName()) + " (connecting...)");

	MonitorHost::connectionLost();

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
//	m_qafclient.sendMsg( new af::Msg( af::Msg::TUserIdRequest, &m_mcuserhost, true));
}

void Dialog::newMessage( af::Msg *msg)
{
	if( msg == NULL)
	{
		AF_ERR << "msg == NULL";
		return;
	}
    #ifdef AFOUTPUT
    printf(" >>> Dialog::newMessage: ");msg->v_stdOut();
    #endif
    switch( msg->type())
    {
    case af::Msg::TVersionMismatch:
    {
		AF_WARN << "Server version mismatch, exiting.";
        emit stop();
        break;
    }
	case af::Msg::TMonitor:
	{
		af::Monitor monitor( msg);

		idReceived( monitor.getId(), monitor.getUid());

		break;
	}
    case af::Msg::TMonitorId:
    {
		idReceived( msg->int32());
        break;
    }
	case af::Msg::TInfo:
	{
		std::string kind, info;
		if( msg->getInfo( kind, info))
		{
			QString qinfo = afqt::stoq( info);
			if( kind == "info")
				displayInfo( qinfo);
			else if( kind == "warning")
				displayWarning( qinfo);
			else if( kind == "error")
				displayError( qinfo);
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
    case af::Msg::TString:
    {
        std::string str = msg->getString();
        if( LabelVersion::getStringStatus( str) != LabelVersion::SS_None )
            announce( str);
        else
			new WndText("Message", msg);
        break;
    }
	case af::Msg::TTaskFiles:
	{
		af::MCTaskUp taskup( msg);
		Watch::filesReceived( taskup);
        break;
	}
	case af::Msg::TJSON:
	case af::Msg::TJSONBIN:
	case af::Msg::THTTP:
	case af::Msg::THTTPGET:
	{
		static int unused;
		unused = ::write( 1, " >>> ", 5);
		msg->stdOutData( false);
		unused = ::write( 1, "\n", 1);
		break;
	}
    default:
        Watch::caseMessage( msg);
    }
    delete msg;
}

void Dialog::idReceived( int i_id, int i_uid)
{
	if( MonitorHost::id() > 0 )
	{
		if( i_id != MonitorHost::id())
		{
			connectionLost();
		}
	}
	else
	{
		if( i_id == 0)
		{
			connectionLost();
		}
		else
		{
			AFINFA("Dialog::idReceived: ID=%d UID=%d\n", i_id, i_uid)

			MonitorHost::connectionEstablished( i_id, i_uid);
			connectionEstablished();
			Watch::connectionEstablished();

			if( i_uid == -1 )
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
				AF_LOG << "Registered, ID=" << i_id;
				if( m_monitorType == Watch::WNONE )
				{
					ButtonMonitor::pushButton( Watch::WJobs);
				}
			}

			af::Msg * msg = new af::Msg( af::Msg::TMonitorUpdateId, i_id);
			m_qafclient.setUpMsg( msg, af::Environment::getWatchGetEventsSec());
		}
	}
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

	if(( type == Watch::WJobs) && ( MonitorHost::getUid() < 0 ) && ( af::Environment::VISOR() == false ))
   {
      ButtonMonitor::unset();
      displayWarning("You are not registered (have no jobs).");
//		m_qafclient.sendMsg( new af::Msg( af::Msg::TUserIdRequest, &m_mcuserhost, true));
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

   if( af::Environment::checkKey( key.at(0).toLatin1()))
   {
      if( af::Environment::GOD())
      {
			MonitorHost::setUid(0);

         m_btnMonitor[Watch::WMonitors] = new ButtonMonitor( Watch::WMonitors, this);
         m_hlayout_b->addWidget( m_btnMonitor[Watch::WMonitors]);

			m_topleft->setText("GOD MODE");
      }
      else if( af::Environment::VISOR())
      {
			MonitorHost::setUid(0);

			m_topleft->setText("VISOR MODE");
      }
      else
      {
			MonitorHost::setUid(-1);

         if( m_btnMonitor[Watch::WMonitors])
         {
            delete m_btnMonitor[Watch::WMonitors];
            m_btnMonitor[Watch::WMonitors] = NULL;
         }
			m_topleft->setText("");
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

void Dialog::actNotifications()
{
    new WndNotifications();
    Watch::displayInfo("Opening 'Notifications' Window");
}

void Dialog::actSavePreferencesOnExit()   { afqt::QEnvironment::savePrefsOnExit.n    = 1 - afqt::QEnvironment::savePrefsOnExit.n;     }
void Dialog::actSaveGUIOnExit()           { afqt::QEnvironment::saveGUIOnExit.n      = 1 - afqt::QEnvironment::saveGUIOnExit.n;       }
void Dialog::actSaveHotkeysOnExit()       { afqt::QEnvironment::saveHotkeysOnExit.n  = 1 - afqt::QEnvironment::saveHotkeysOnExit.n;   }
void Dialog::actSaveWndRectsOnExit()      { afqt::QEnvironment::saveWndRectsOnExit.n = 1 - afqt::QEnvironment::saveWndRectsOnExit.n;  }
void Dialog::actShowOfflineNoise()        { afqt::QEnvironment::showOfflineNoise.n   = 1 - afqt::QEnvironment::showOfflineNoise.n;    }
void Dialog::actShowDocs()  { Watch::showDocs();  }
void Dialog::actShowForum() { Watch::showForum(); }

void Dialog::actSavePreferences()
{
   if( afqt::QEnvironment::save())
      Watch::displayInfo(QString("Saved '%1'").arg( afqt::QEnvironment::getFileName()));
   else
      Watch::displayError(QString("Failed to save to'%1'").arg( afqt::QEnvironment::getFileName()));
}

void Dialog::actGuiLevel( int i_level)
{
	QString message;

	switch( i_level)
	{
		case AFGUI::PADAWAN:
			menuBar()->show();
			message = "Patience you must have. My young Padawan.";
			break;
		case AFGUI::JEDI:
			menuBar()->show();
			message = "May the force be with you.";
			break;
		case AFGUI::SITH:
			menuBar()->hide();
			Watch::displayInfo("Welcome to the dark side.");
			message = "Powerful you have become, the dark side I sense in you.";
			break;
		default:
			Watch::displayError(QString("Invalid theme number: %1").arg( i_level));
			return;
	}

	//Watch::displayInfo( message);
	Watch::notify( message);

	afqt::QEnvironment::level.n = i_level;
	Watch::refreshGui();
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
