#include "listrenders.h"

#include "../include/afanasy.h"

#include "../libafanasy/address.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/monitor.h"
#include "../libafanasy/monitorevents.h"
#include "../libafanasy/msgclasses/mctaskpos.h"

#include "actionid.h"
#include "dialog.h"
#include "buttonpanel.h"
#include "ctrlrenders.h"
#include "ctrlsortfilter.h"
#include "itemrender.h"
#include "modelnodes.h"
#include "viewitems.h"
#include "watch.h"
#include "wndtask.h"

#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtGui/QContextMenuEvent>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QMenu>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ListRenders::EDisplaySize ListRenders::ms_displaysize = ListRenders::EVariableSize;

int     ListRenders::ms_SortType1      = CtrlSortFilter::TPRIORITY;
int     ListRenders::ms_SortType2      = CtrlSortFilter::TCAPACITY;
bool    ListRenders::ms_SortAscending1 = false;
bool    ListRenders::ms_SortAscending2 = false;
int     ListRenders::ms_FilterType     = CtrlSortFilter::TNAME;
bool    ListRenders::ms_FilterInclude  = true;
bool    ListRenders::ms_FilterMatch    = false;
std::string ListRenders::ms_FilterString = "";

ListRenders::ListRenders( QWidget* parent):
	ListNodes( parent, "renders")
{
	m_ctrl_sf = new CtrlSortFilter( this,
			&ms_SortType1, &ms_SortAscending1,
			&ms_SortType2, &ms_SortAscending2,
			&ms_FilterType, &ms_FilterInclude, &ms_FilterMatch, &ms_FilterString
		);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TNONE);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TPRIORITY);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TCAPACITY);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TELDERTASKTIME);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TTIMELAUNCHED);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TTIMEREGISTERED);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TNAME);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TTASKUSER);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TUSERNAME);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TENGINE);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TADDRESS);
	m_ctrl_sf->addFilterType( CtrlSortFilter::TNONE);
	m_ctrl_sf->addFilterType( CtrlSortFilter::TNAME);
	m_ctrl_sf->addFilterType( CtrlSortFilter::TUSERNAME);
	m_ctrl_sf->addFilterType( CtrlSortFilter::TTASKUSER);
	m_ctrl_sf->addFilterType( CtrlSortFilter::TENGINE);
	m_ctrl_sf->addFilterType( CtrlSortFilter::TADDRESS);
	initSortFilterCtrl();

	CtrlRenders * control = new CtrlRenders( m_ctrl_sf, this);
	m_ctrl_sf->getLayout()->addWidget( control);

	// Add left panel buttons:
	ButtonPanel * bp;

	bp = addButtonPanel("LOG","renders_log","Show render log.");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actRequestLog()));

	bp = addButtonPanel("TLG","renders_tasks_log","Show tasks log.");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actRequestTasksLog()));

	bp = addButtonPanel("nim","renders_nimby","Set nimby.","M");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actNimby()));

	bp = addButtonPanel("NIM","renders_NIMBY","Set NIMBY.","N");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actNIMBY()));

	bp = addButtonPanel("FRE","renders_free","Set free.","F");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actFree()));

	bp = addButtonPanel("EJA","renders_eject_all","Eject all tasks.","", true);
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actEjectTasks()));

	bp = addButtonPanel("EJN","renders_eject_notmy","Eject not my tasks.","", true);
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actEjectNotMyTasks()));

	if( af::Environment::GOD())
	{
		bp = addButtonPanel("PAU","renders_pause","Pause selected renders.","P");
		connect( bp, SIGNAL( sigClicked()), this, SLOT( actSetPaused()));

		bp = addButtonPanel("STA","renders_unpause","Start (Unpause) selected renders.","S");
		connect( bp, SIGNAL( sigClicked()), this, SLOT( actUnsetPaused()));
	}


	timer = new QTimer( this);
	connect(timer, SIGNAL(timeout()), this, SLOT( requestResources()));
	timer->start( 990 * af::Environment::getRenderHeartbeatSec() * af::Environment::getRenderUpResourcesPeriod());

	m_parentWindow->setWindowTitle("Renders");

	init();

	connect( (ModelNodes*)m_model, SIGNAL(   nodeAdded( ItemNode *, const QModelIndex &)),
	                         this,   SLOT( renderAdded( ItemNode *, const QModelIndex &)));

	if( false == af::Environment::VISOR())
		connect( m_view->selectionModel(), SIGNAL( selectionChanged( const QItemSelection &, const QItemSelection &)),
	                                 this,   SLOT( selectionChanged( const QItemSelection &, const QItemSelection &)));

	setSpacing();

	QTimer * timer = new QTimer(this);
	timer->start( 1900 * af::Environment::getWatchRefreshGuiSec());
	connect( timer, SIGNAL( timeout()), this, SLOT( repaintItems()));
}

ListRenders::~ListRenders()
{
AFINFO("ListRenders::~ListRenders.")
}

void ListRenders::setSpacing()
{
	 switch( ms_displaysize )
	 {
	 case  ListRenders::ESMallSize:
		  m_view->setSpacing( 1);
		  break;
	 case  ListRenders::ENormalSize:
		  m_view->setSpacing( 2);
		  break;
	 default:
		  m_view->setSpacing( 3);
	 }
}

void ListRenders::actChangeSize( int i_size)
{
	 ListRenders::EDisplaySize dsize = (ListRenders::EDisplaySize)i_size;

	 if( dsize == ms_displaysize )
		  return;

	 ms_displaysize = dsize;

	 setSpacing();
	 itemsHeightCalc();
	 revertModel();
	 repaintItems();
}

void ListRenders::renderAdded( ItemNode * node, const QModelIndex & index)
{
	ItemRender * render = (ItemRender*)node;
	if( af::Environment::VISOR() == false)
	{
	   if(( render->getName() == QString::fromUtf8( af::Environment::getComputerName().c_str())) || (render->getUserName() == QString::fromUtf8( af::Environment::getUserName().c_str())))
			m_view->selectionModel()->select( index, QItemSelectionModel::Select);
	}
}

void ListRenders::selectionChanged( const QItemSelection & selected, const QItemSelection & deselected )
{
	QModelIndexList indexes = selected.indexes();
	for( int i = 0; i < indexes.count(); i++)
		if( Item::isItemP( indexes[i].data()))
		{
			ItemRender * render = (ItemRender*)(Item::toItemP( indexes[i].data()));
			if(( render->getName() != QString::fromUtf8( af::Environment::getComputerName().c_str())) && ( render->getUserName() != QString::fromUtf8( af::Environment::getUserName().c_str())))
				m_view->selectionModel()->select( indexes[i], QItemSelectionModel::Deselect);
		}
}

void ListRenders::requestResources()
{
	af::MCGeneral ids;
	for( int i = 0; i < m_model->count(); i++)
	{
		ItemRender *render = (ItemRender*)(m_model->item(i));
		if( render == NULL) continue;
		if( false == render->isOnline()) continue;
		ids.addId( render->getId());
	}

	if( ids.getCount())
	{
		Watch::sendMsg( new af::Msg( af::Msg::TRendersResourcesRequestIds, &ids));
	}
}

void ListRenders::contextMenuEvent( QContextMenuEvent *event)
{
	ItemRender* render = (ItemRender*)getCurrentItem();
	if( render == NULL ) return;
	bool me = false;
	if( render->getName().contains( QString::fromUtf8( af::Environment::getComputerName().c_str())) || ( render->getUserName() == QString::fromUtf8( af::Environment::getUserName().c_str()))) me = true;
	int selectedItemsCount = getSelectedItemsCount();

	QMenu menu(this);
	QAction *action;
	QMenu * submenu;

	if( selectedItemsCount <= 1 )
	{
		action = new QAction( render->getName(), this);
		action->setEnabled( false);
		menu.addAction( action);
		menu.addSeparator();
	}

	action = new QAction( "Show Log", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actRequestLog() ));
	menu.addAction( action);

	action = new QAction( "Tasks Log", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actRequestTasksLog() ));
	menu.addAction( action);

	action = new QAction( "Show Info", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actRequestInfo() ));
	menu.addAction( action);
	
	std::list<const af::TaskExec*> l = render->getTasks();
	submenu = new QMenu( l.size() > 1 ? "Running Tasks" : "Running Task", this);
	submenu->setEnabled( render->hasTasks());

	std::list<const af::TaskExec*>::const_iterator it;
	for (it = l.begin() ; it != l.end() ; ++it)
	{
		const af::TaskExec *task = *it;
		QString title = QString("%1[%2][%3]")
			.arg( QString::fromStdString(task->getJobName()))
			.arg( QString::fromStdString(task->getBlockName()))
			.arg( QString::fromStdString(task->getName()));

		action = new ActionIdIdId(
			task->getJobId(),
			task->getBlockNum(),
			task->getTaskNum(),
			title,
			this);

		connect( action, SIGNAL( triggeredId(int,int,int) ),
				 this, SLOT( actRequestTaskInfo(int,int,int) ));

		submenu->addAction( action);
	}
	
	menu.addMenu( submenu);

	if( me || af::Environment::VISOR())
	{
		menu.addSeparator();

		action = new QAction( "Set NIMBY", this);
		if( selectedItemsCount == 1) action->setEnabled(false == render->isNIMBY());
		connect( action, SIGNAL( triggered() ), this, SLOT( actNIMBY() ));
		menu.addAction( action);
		action = new QAction( "Set nimby", this);
		if( selectedItemsCount == 1) action->setEnabled(false == render->isNimby());
		connect( action, SIGNAL( triggered() ), this, SLOT( actNimby() ));
		menu.addAction( action);
		action = new QAction( "Set Free", this);
		if( selectedItemsCount == 1) action->setEnabled(render->isNimby() || render->isNIMBY());
		connect( action, SIGNAL( triggered() ), this, SLOT( actFree() ));
		menu.addAction( action);
		action = new QAction( "Set User", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actUser() ));
		menu.addAction( action);

		menu.addSeparator();

		submenu = new QMenu( "Eject", this);

		action = new QAction( "All Tasks", this);
		if( selectedItemsCount == 1) action->setEnabled( render->hasTasks());
		connect( action, SIGNAL( triggered() ), this, SLOT( actEjectTasks() ));
		submenu->addAction( action);

		action = new QAction( "Not My Tasks", this);
		if( selectedItemsCount == 1) action->setEnabled( render->hasTasks());
		connect( action, SIGNAL( triggered() ), this, SLOT( actEjectNotMyTasks() ));
		submenu->addAction( action);

		menu.addMenu( submenu);
	}

	QMenu * custom_submenu = NULL;
	int custom_cmd_index = 0;
	if(af::Environment::getRenderCmds().size() > 0)
	{
		menu.addSeparator();
		custom_submenu = new QMenu( "Custom", this);
		for( std::vector<std::string>::const_iterator it = af::Environment::getRenderCmds().begin(); it != af::Environment::getRenderCmds().end(); it++, custom_cmd_index++)
		{
			ActionId * actionid = new ActionId( custom_cmd_index, QString("%1").arg( afqt::stoq(*it)), this);
			connect( actionid, SIGNAL( triggeredId( int ) ), this, SLOT( actCommand( int ) ));
			custom_submenu->addAction( actionid);
		}
		menu.addMenu( custom_submenu);
	}
	if( af::Environment::GOD() && ( af::Environment::getRenderCmdsAdmin().size() > 0 ))
	{
		if( custom_submenu == NULL)
		{
			menu.addSeparator();
			custom_submenu = new QMenu( "Custom", this);
		}
	  for( std::vector<std::string>::const_iterator it = af::Environment::getRenderCmdsAdmin().begin(); it != af::Environment::getRenderCmdsAdmin().end(); it++, custom_cmd_index++)
		{
			ActionId * actionid = new ActionId( custom_cmd_index, QString("%1").arg( afqt::stoq(*it)), this);
			connect( actionid, SIGNAL( triggeredId( int ) ), this, SLOT( actCommand( int ) ));
			custom_submenu->addAction( actionid);
		}
		menu.addMenu( custom_submenu);
	}

	// Menu completed for user mode:

	if( ! af::Environment::GOD())
	{
		menu.exec( event->globalPos());
		return;
	}
		
	// Administare ( super user mode ):

	menu.addSeparator();

	submenu = new QMenu("Administrate", this);
	menu.addMenu( submenu);

	action = new QAction( "Annotate", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actAnnotate() ));
	submenu->addAction( action);
	action = new QAction( "Set Priority", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actPriority() ));
	submenu->addAction( action);

	submenu->addSeparator();

	action = new QAction("Set Paused", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actSetPaused() ));
	submenu->addAction( action);
	action = new QAction("Unset Paused", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actUnsetPaused() ));
	submenu->addAction( action);

	submenu->addSeparator();

	action = new QAction( "Change Capacity", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actCapacity() ));
	submenu->addAction( action);
	action = new QAction( "Change Max Tasks", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actMaxTasks() ));
	submenu->addAction( action);
	action = new QAction( "Enable Service", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actEnableService() ));
	submenu->addAction( action);
	action = new QAction( "Disable Service", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actDisableService() ));
	submenu->addAction( action);
	action = new QAction( "Restore Defaults", this);
	if( selectedItemsCount == 1) action->setEnabled(render->isDirty());
	connect( action, SIGNAL( triggered() ), this, SLOT( actRestoreDefaults() ));
	submenu->addAction( action);

	submenu->addSeparator();

	action = new QAction("Launch Command", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actLaunchCmd() ));
	if( selectedItemsCount == 1) action->setEnabled( render->isOnline());
	submenu->addAction( action);
	action = new QAction("Launch And Exit", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actLaunchCmdExit() ));
	if( selectedItemsCount == 1) action->setEnabled( render->isOnline());
	submenu->addAction( action);

	submenu->addSeparator();

	action = new QAction("Exit Client", this);
	if( selectedItemsCount == 1) action->setEnabled(render->isOnline());
	connect( action, SIGNAL( triggered() ), this, SLOT( actExit() ));
	submenu->addAction( action);
	action = new QAction("Delete Client", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actDelete() ));
	if( selectedItemsCount == 1) action->setEnabled(false == render->isOnline());
	submenu->addAction( action);

	submenu->addSeparator();

	action = new QAction("Wake-On-Lan Sleep", this);
	if( selectedItemsCount == 1)
		action->setEnabled( render->isOnline() && ( false == render->isBusy()) && ( false == render->isWOLFalling()));
	connect( action, SIGNAL( triggered() ), this, SLOT( actWOLSleep()));
	submenu->addAction( action);
	action = new QAction("Wake-On-Lan Wake", this);
	if( selectedItemsCount == 1) action->setEnabled( render->isOffline());
	connect( action, SIGNAL( triggered() ), this, SLOT( actWOLWake()));
	submenu->addAction( action);

	submenu->addSeparator();

	action = new QAction("Reboot Machine", this);
	if( selectedItemsCount == 1) action->setEnabled(render->isOnline());
	connect( action, SIGNAL( triggered() ), this, SLOT( actReboot() ));
	submenu->addAction( action);
	action = new QAction("Shutdown Machine", this);
	if( selectedItemsCount == 1) action->setEnabled(render->isOnline());
	connect( action, SIGNAL( triggered() ), this, SLOT( actShutdown() ));
	submenu->addAction( action);

	menu.exec( event->globalPos());
}

void ListRenders::doubleClicked( Item * item)
{
	actRequestInfo();
}

bool ListRenders::v_caseMessage( af::Msg * msg)
{
#ifdef AFOUTPUT
	msg->stdOut();
#endif
	switch( msg->type())
	{
	case af::Msg::TRendersList:
		subscribe();
	case af::Msg::TRendersResources:
	{
		updateItems( msg);
		calcTitle();
		break;
	}
	default:
		return false;
	}
	return true;
}

bool ListRenders::v_processEvents( const af::MonitorEvents & i_me)
{
	if( i_me.m_events[af::Monitor::EVT_renders_del].size())
	{
		deleteItems( i_me.m_events[af::Monitor::EVT_renders_del]);
		calcTitle();
		return true;
	}

	std::vector<int> ids;

	for( int i = 0; i < i_me.m_events[af::Monitor::EVT_renders_change].size(); i++)
		af::addUniqueToVect( ids, i_me.m_events[af::Monitor::EVT_renders_change][i]);

	for( int i = 0; i < i_me.m_events[af::Monitor::EVT_renders_add].size(); i++)
		af::addUniqueToVect( ids, i_me.m_events[af::Monitor::EVT_renders_add][i]);

	if( ids.size())
	{
		get( ids);
		return true;
	}

	return false;
}

ItemNode* ListRenders::v_createNewItem( af::Node * i_node, bool i_subscibed)
{
	return new ItemRender( (af::Render*)i_node, m_ctrl_sf);
}

void ListRenders::calcTitle()
{
	int total = count();
	int online = 0;
	int busy = 0;
	int nimby = 0;
	int free = 0;
	for( int i = 0; i < total; i++)
	{
		ItemRender * itemrender = (ItemRender*)(m_model->item(i));
		if( itemrender->isOnline()) online++;
		if( itemrender->isBusy()) busy++;
		if( itemrender->isNimby() || itemrender->isNIMBY()) nimby++;
		else if( itemrender->isOnline() && (false == itemrender->isBusy())) free++;
	}
	m_parentWindow->setWindowTitle(QString("R[%1/%2]: B%3/%4F (n%5)").arg( total).arg( online).arg( busy).arg( free).arg( nimby));
}

void ListRenders::actCapacity()
{
	ItemRender* item = (ItemRender*)getCurrentItem();
	if( item == NULL ) return;
	int current = item->getCapacity();

	bool ok;
	int capacity = QInputDialog::getInt(this, "Change Capacity", "Enter New Capacity", current, -1, 1000000, 1, &ok);
	if( !ok) return;
	setParameter("capacity", capacity);
}
void ListRenders::actMaxTasks()
{
	ItemRender* item = (ItemRender*)getCurrentItem();
	if( item == NULL ) return;
	int current = item->getMaxTasks();

	bool ok;
	int max_tasks = QInputDialog::getInt(this, "Change Maximum Tasksy", "Enter New Limit", current, -1, 1000000, 1, &ok);
	if( !ok) return;
	setParameter("max_tasks", max_tasks);
}

void ListRenders::actNIMBY()       { setParameter("NIMBY",  "true",  false); }
void ListRenders::actNimby()       { setParameter("nimby",  "true",  false); }
void ListRenders::actFree()        { setParameter("nimby",  "false", false); }
void ListRenders::actSetPaused()   { setParameter("paused", "true",  false); }
void ListRenders::actUnsetPaused() { setParameter("paused", "false", false); }

void ListRenders::actUser()
{
	QString current = afqt::stoq( af::Environment::getUserName());

	bool ok;
	QString text = QInputDialog::getText(this, "Set User", "Enter User Name", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	setParameter("user_name", afqt::qtos( text));
}

void ListRenders::actEjectTasks()      { operation("eject_tasks"); }
void ListRenders::actEjectNotMyTasks() { operation("eject_tasks_keep_my"); }
void ListRenders::actExit()            { operation("exit"); }
void ListRenders::actDelete()          { operation("delete"); }
void ListRenders::actReboot()          { operation("reboot"); }
void ListRenders::actShutdown()        { operation("shutdown"); }
void ListRenders::actWOLSleep()        { operation("wol_sleep"); }
void ListRenders::actWOLWake()         { operation("wol_wake"); }
void ListRenders::actRestoreDefaults() { operation("restore_defaults"); }

void ListRenders::actRequestLog()      { getItemInfo("log"); }
void ListRenders::actRequestTasksLog() { getItemInfo("tasks_log"); }
void ListRenders::actRequestInfo()     { getItemInfo("full"); }

void ListRenders::actRequestTaskInfo(int jid, int bnum, int tnum)
{
	WndTask::openTask( af::MCTaskPos( jid, bnum, tnum));
}

void ListRenders::actEnableService()  { setService( true );}
void ListRenders::actDisableService() { setService( false);}
void ListRenders::setService( bool enable)
{
	Item* item = getCurrentItem();
	if( item == NULL ) return;
	QString caption("Service");
	if( enable ) caption = "Enable " + caption; else caption = "Disable " + caption;

	bool ok;
	QString service_mask = QInputDialog::getText(this, caption, "Enter Service Name", QLineEdit::Normal, QString(), &ok);
	if( !ok) return;

	std::string err;
	if( false == af::RegExp::Validate( afqt::qtos( service_mask), &err))
	{
		displayError( afqt::stoq( err));
		return;
	}
	
	std::ostringstream str;
	af::jsonActionOperationStart( str, "renders", "service", "", getSelectedIds());
	str << ",\n\"name\":\"" << afqt::qtos( service_mask) << "\"";
	str << ",\n\"enable\":" << ( enable ? "true": "false" );
	af::jsonActionOperationFinish( str);
	Watch::sendMsg( af::jsonMsg( str));
}

void ListRenders::actCommand( int number)
{
	std::list<std::string> commands;
	// Create a list that contains and user and admin commands:
	for( std::vector<std::string>::const_iterator it = af::Environment::getRenderCmds().begin(); it != af::Environment::getRenderCmds().end(); it++)
		commands.push_back( *it);
	for( std::vector<std::string>::const_iterator it = af::Environment::getRenderCmdsAdmin().begin(); it != af::Environment::getRenderCmdsAdmin().end(); it++)
		commands.push_back( *it);

	if( number >= commands.size())
	{
		displayError( "No such command.");
		return;
	}

	QModelIndexList indexes( m_view->selectionModel()->selectedIndexes());

	std::list<std::string>::const_iterator it = commands.begin();
	for( int i = 0; i < number; i++ ) it++;
	QString cmd( afqt::stoq(*it));

	if( cmd.contains( AFWATCH::CMDS_ASKCOMMAND))
	{
		bool ok;
		QString text = QInputDialog::getText(this, "Launch Command",
			QString("Enter string to replace %1 in\n%2").arg(AFWATCH::CMDS_ASKCOMMAND).arg(cmd), QLineEdit::Normal, "", &ok);
		if( !ok) return;
		cmd.replace( AFWATCH::CMDS_ASKCOMMAND, text);
	}

	if( indexes.count() < 1 )
	{
		ItemRender * item = (ItemRender*)(getCurrentItem());
		cmd.replace( AFWATCH::CMDS_ARGUMENT, item->getName());
		cmd.replace( AFWATCH::CMDS_IPADDRESS, item->getIPString());
		Watch::startProcess( cmd);
		return;
	}

	for( int i = 0; i < indexes.count(); i++)
	{
		if( false == Item::isItemP( indexes[i].data())) continue;
		ItemRender * item = (ItemRender*)(Item::toItemP( indexes[i].data()));
		if( item == NULL ) continue;
		QString final_command(cmd);
		final_command.replace( AFWATCH::CMDS_ARGUMENT, item->getName());
		final_command.replace( AFWATCH::CMDS_IPADDRESS, item->getIPString());
		Watch::startProcess( final_command);
	}
}

void ListRenders::actLaunchCmd()     { launchCmdExit( false);}
void ListRenders::actLaunchCmdExit() { launchCmdExit( true );}
void ListRenders::launchCmdExit( bool i_exit)
{
	Item* item = getCurrentItem();
	if( item == NULL ) return;

	QString caption("Launch Command");
	if( i_exit ) caption += "And Exit";

	bool ok;
	QString cmd = QInputDialog::getText( this, caption,"Enter Command", QLineEdit::Normal, QString(), &ok);
	if( !ok) return;

	std::ostringstream str;
	af::jsonActionOperationStart( str, "renders", "launch_cmd", "", getSelectedIds());
	str << ",\n\"cmd\":\"" << afqt::qtos( cmd) << "\"";
	if( i_exit )
		str << ",\n\"exit\":true";
	af::jsonActionOperationFinish( str);
	Watch::sendMsg( af::jsonMsg( str));
}


