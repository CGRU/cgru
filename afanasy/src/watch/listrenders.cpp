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
#include "itempool.h"
#include "modelnodes.h"
#include "paramspanelfarm.h"
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
#include "../libafanasy/logger.h"

ListRenders::EDisplaySize ListRenders::ms_displaysize = ListRenders::EVariableSize;

int     ListRenders::ms_SortType1      = CtrlSortFilter::TPRIORITY;
int     ListRenders::ms_SortType2      = CtrlSortFilter::TNAME;
bool    ListRenders::ms_SortAscending1 = false;
bool    ListRenders::ms_SortAscending2 = true;
int     ListRenders::ms_FilterType     = CtrlSortFilter::TNAME;
bool    ListRenders::ms_FilterInclude  = true;
bool    ListRenders::ms_FilterMatch    = false;
std::string ListRenders::ms_FilterString = "";

ListRenders::ListRenders( QWidget* parent):
	ListNodes( parent, "renders")
{
	m_node_types.clear();
	m_node_types.push_back("pools");
	m_node_types.push_back("renders");

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
	ButtonPanel * bp; ButtonsMenu * bm;

	bp = addButtonPanel(Item::TAny, "LOG","renders_log","Get render log.");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actRequestLog()));

	if( af::Environment::GOD())
	{
		bp = addButtonPanel(Item::TPool, "ADD POOL","pools_add","Add a new child pool.");
		connect(bp, SIGNAL(sigClicked()), this, SLOT(actAddPool()));
	}

	bp = addButtonPanel(Item::TRender, "TASKS LOG","renders_tasks_log","Get tasks log.");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actRequestTasksLog()));

	bp = addButtonPanel(Item::TRender, "nimby","renders_nimby","Set nimby.","M");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actNimby()));

	bp = addButtonPanel(Item::TRender, "NIMBY","renders_NIMBY","Set NIMBY.","N");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actNIMBY()));

	bp = addButtonPanel(Item::TRender, "FREE","renders_free","Set free.","F");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actFree()));

	bm = addButtonsMenu(Item::TRender, "Eject Tasks","Eject tasks from render.");

	bp = addButtonPanel(Item::TAny, "ALL","renders_eject_all","Eject all tasks.","", true);
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actEjectTasks()));

	bp = addButtonPanel(Item::TAny, "NOT MY","renders_eject_notmy","Eject not my tasks.","", true);
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actEjectNotMyTasks()));

	resetButtonsMenu();

	if( af::Environment::GOD())
	{
		bm = addButtonsMenu(Item::TRender, "Pool","Change render pool.");

		bp = addButtonPanel(Item::TRender, "SET","renders_pool_set","Set render pool.","", false);
		connect(bp, SIGNAL(sigClicked()), this, SLOT(actRenderSetPool()));

		bp = addButtonPanel(Item::TRender, "REASSIGN","renders_pool_reassign","Re-assign render pool.","", true);
		connect(bp, SIGNAL(sigClicked()), this, SLOT(actRenderReAssing()));

		resetButtonsMenu();

		bm = addButtonsMenu(Item::TAny, "Services","Add/Remove/Disable service(s).");

		bp = addButtonPanel(Item::TAny, "ADD","service_add","Add a service.","", false);
		connect(bp, SIGNAL(sigClicked()), this, SLOT(slot_ServiceAdd()));

		bp = addButtonPanel(Item::TAny, "REMOVE","service_remove","Remove services by mask.","", false);
		connect(bp, SIGNAL(sigClicked()), this, SLOT(actServiceRemove()));

		bp = addButtonPanel(Item::TAny, "ENABLE","service_enable","Enable services by mask.","", false);
		connect(bp, SIGNAL(sigClicked()), this, SLOT(actServiceEnable()));

		bp = addButtonPanel(Item::TAny, "DISABLE","service_disable","Disable services by mask.","", false);
		connect(bp, SIGNAL(sigClicked()), this, SLOT(slot_ServiceDisable()));

		bp = addButtonPanel(Item::TAny, "CLEAR","clear_services",
				"Clear services.\nServices settings will be taken from the parent pool.","", true);
		connect(bp, SIGNAL(sigClicked()), this, SLOT(actClearServices()));

		resetButtonsMenu();

		bm = addButtonsMenu(Item::TPool, "New Render","New render default state.");

		bp = addButtonPanel(Item::TPool, "Nimby","new_render_nimby","New render will be registered in NIMBY state.","", true);
		connect(bp, SIGNAL(sigClicked()), this, SLOT(actNewRenderNimby()));

		bp = addButtonPanel(Item::TPool, "Free","new_render_free","New render will NOT be registered in NIMBY state.","", true);
		connect(bp, SIGNAL(sigClicked()), this, SLOT(actNewRenderFree()));

		bp = addButtonPanel(Item::TPool, "Paused","new_render_paused","New render will be registered in Paused state.","", true);
		connect(bp, SIGNAL(sigClicked()), this, SLOT(actNewRenderPaused()));

		bp = addButtonPanel(Item::TPool, "Ready","new_render_free","New render will NOT be registered in Paused state.","", true);
		connect(bp, SIGNAL(sigClicked()), this, SLOT(actNewRenderReady()));

		resetButtonsMenu();

		bp = addButtonPanel(Item::TRender, "PAUSE","renders_pause","Pause selected renders.","P");
		connect( bp, SIGNAL( sigClicked()), this, SLOT( actSetPaused()));

		bp = addButtonPanel(Item::TRender, "START","renders_unpause","Start (Unpause) selected renders.","S");
		connect( bp, SIGNAL( sigClicked()), this, SLOT( actUnsetPaused()));

		bp = addButtonPanel(Item::TAny, "DELETE","renders_delete","Delete pool or render.","", true);
		connect(bp, SIGNAL(sigClicked()), this, SLOT(actDelete()));
	}


	timer = new QTimer( this);
	connect(timer, SIGNAL(timeout()), this, SLOT( requestResources()));
	timer->start( 990 * af::Environment::getRenderHeartbeatSec() * af::Environment::getRenderUpResourcesPeriod());

	m_parentWindow->setWindowTitle("Renders");

	if (af::Environment::GOD())
	{
		addParam_REx(Item::TPool, "pattern",        "Pattern",        "Host names pattern regular expression");
		addParam_Num(Item::TAny,  "priority",       "Priority",       "Priority number", 0, 250);
		addParam_Num(Item::TPool, "host_max_tasks", "Host Max Tasks", "Pool hosts Maximum running tasks", -1, 99);
		addParam_Num(Item::TPool, "host_capacity",  "Host Capacity",  "Pool hosts Capacity", -1, 1<<30);
		addParam_Str(Item::TAny,  "annotation",     "Annotation",     "Annotation string");
	}

	ParamsPanelFarm * paramspanelfarm = new ParamsPanelFarm(this);
	m_paramspanel = paramspanelfarm;
	initListNodes();

	connect( (ModelNodes*)m_model, SIGNAL(   nodeAdded( ItemNode *, const QModelIndex &)),
	                         this,   SLOT( renderAdded( ItemNode *, const QModelIndex &)));

	if (false == af::Environment::VISOR())
		connect(m_view->selectionModel(), SIGNAL(       selectionChanged(const QItemSelection &, const QItemSelection &)),
	                                this,   SLOT(rendersSelectionChanged(const QItemSelection &, const QItemSelection &)));

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
	 case  ListRenders::ESmallSize:
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
	if (af::Environment::VISOR() == false)
	{
	   if (render->getName() == QString::fromUtf8(af::Environment::getComputerName().c_str()))
		{
		   m_view->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
		}
	}
}

void ListRenders::rendersSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
	QModelIndexList indexes = selected.indexes();
	if (indexes.size() == 0)
	{
		return;
	}

	for (int i = 0; i < indexes.count(); i++)
		if (Item::isItemP(indexes[i].data()))
		{
			Item * item = (Item::toItemP(indexes[i].data()));

			if (item->getType() == Item::TRender)
			{
				ItemRender * render = (ItemRender*)item;
				if ((render->getName() != QString::fromUtf8(af::Environment::getComputerName().c_str())) &&
					(render->getUserName() != QString::fromUtf8(af::Environment::getUserName().c_str())))
					m_view->selectionModel()->select(indexes[i], QItemSelectionModel::Deselect);
			}
			else
				m_view->selectionModel()->select(indexes[i], QItemSelectionModel::Deselect);
		}
}

void ListRenders::requestResources()
{
	af::MCGeneral ids;
	for( int i = 0; i < m_model->count(); i++)
	{
		Item * item = m_model->item(i);
		if (item == NULL)
			continue;

		if (item->getType() != Item::TRender)
			continue;

		ItemRender * render = (ItemRender*)item;
		if (render->isOnline())
			ids.addId(render->getId());
	}

	if( ids.getCount())
	{
		Watch::sendMsg( new af::Msg( af::Msg::TRendersResourcesRequestIds, &ids));
	}
}

void ListRenders::contextMenuEvent( QContextMenuEvent *event)
{
	Item * item = getCurrentItem();
	if (item == NULL)
		return;

	if (item->getType() != Item::TRender)
		return;

	ItemRender * render = (ItemRender*)item;

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
	  for (std::vector<std::string>::const_iterator it = af::Environment::getRenderCmdsAdmin().begin();
			  it != af::Environment::getRenderCmdsAdmin().end();
			  it++, custom_cmd_index++)
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

	submenu->addSeparator();

	action = new QAction("Launch Command ...", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actLaunchCmd() ));
	if( selectedItemsCount == 1) action->setEnabled( render->isOnline());
	submenu->addAction( action);

	if (af::Environment::getRenderLaunchCmds().size())
	{
		QMenu * custom_submenu = new QMenu("Launch Command >>>", this);
		for (
				std::vector<std::string>::const_iterator it = af::Environment::getRenderLaunchCmds().begin();
				it != af::Environment::getRenderLaunchCmds().end();
				it++)
		{
			QString cmd(afqt::stoq(*it));
			QStringList cmdSplit = cmd.split("|");

			ActionString * action_string = new ActionString(cmdSplit.last(), cmdSplit.first(), this);
			connect(action_string, SIGNAL(triggeredString(QString)), this, SLOT(actLaunchCmdString(QString)));
			custom_submenu->addAction(action_string);
		}
		submenu->addMenu(custom_submenu);
	}

	action = new QAction("Launch And Exit ...", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actLaunchCmdExit() ));
	if( selectedItemsCount == 1) action->setEnabled( render->isOnline());
	submenu->addAction( action);

	if (af::Environment::getRenderLaunchCmdsExit().size())
	{
		QMenu * custom_submenu = new QMenu("Launch And Exit >>>", this);
		for (
				std::vector<std::string>::const_iterator it = af::Environment::getRenderLaunchCmdsExit().begin();
				it != af::Environment::getRenderLaunchCmdsExit().end();
				it++)
		{
			QString cmd(afqt::stoq(*it));
			QStringList cmdSplit = cmd.split("|");

			ActionString * action_string = new ActionString(cmdSplit.last(), cmdSplit.first(), this);
			connect(action_string, SIGNAL(triggeredString(QString)), this, SLOT(actLaunchCmdExitString(QString)));
			custom_submenu->addAction(action_string);
		}
		submenu->addMenu(custom_submenu);
	}

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
	case af::Msg::TPoolsList:
	{
		updateItems(msg, Item::TPool);
		calcTitle();
		break;
	}
	case af::Msg::TRendersList:
		subscribe();
	case af::Msg::TRendersResources:
	{
		updateItems(msg, Item::TRender);
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
	bool processed = false;

	// Delete renders by ids:
	if (i_me.m_events[af::Monitor::EVT_renders_del].size())
	{
		deleteItems(i_me.m_events[af::Monitor::EVT_renders_del], Item::TRender);
		calcTitle();
		processed = true;
	}

	// Delete pools by ids:
	if (i_me.m_events[af::Monitor::EVT_pools_del].size())
	{
		deleteItems(i_me.m_events[af::Monitor::EVT_pools_del], Item::TPool);
		calcTitle();
		processed = true;
	}

	// Get new and changed pools ids:
	std::vector<int> pids;
	for (int i = 0; i < i_me.m_events[af::Monitor::EVT_pools_change].size(); i++)
		af::addUniqueToVect(pids, i_me.m_events[af::Monitor::EVT_pools_change][i]);
	for (int i = 0; i < i_me.m_events[af::Monitor::EVT_pools_add].size(); i++)
		af::addUniqueToVect(pids, i_me.m_events[af::Monitor::EVT_pools_add][i]);
	if (pids.size())
	{
		get(pids, "pools");
		processed = true;
	}

	// Get new and changed renders ids:
	std::vector<int> rids;
	for (int i = 0; i < i_me.m_events[af::Monitor::EVT_renders_change].size(); i++)
		af::addUniqueToVect(rids, i_me.m_events[af::Monitor::EVT_renders_change][i]);
	for (int i = 0; i < i_me.m_events[af::Monitor::EVT_renders_add].size(); i++)
		af::addUniqueToVect(rids, i_me.m_events[af::Monitor::EVT_renders_add][i]);
	if (rids.size())
	{
		get(rids, "renders");
		processed = true;
	}

	return processed;
}

ItemNode * ListRenders::v_createNewItemNode(af::Node * i_afnode, Item::EType i_type, bool i_notify)
{
	switch (i_type)
	{
	case Item::TRender:
		return new ItemRender((af::Render*)i_afnode, this, m_ctrl_sf);
	case Item::TPool:
		return new ItemPool((af::Pool*)i_afnode, this, m_ctrl_sf);
	default:
		AF_ERR << "Invalid Item::EType: " << i_type;
		return NULL;
	}
}

void ListRenders::offsetHierarchy(ItemPool * i_item_pool)
{
	// Store pool in "pool name" -> "pool item" map:
	m_pools[i_item_pool->getName()] = i_item_pool;

	// Offset pool:
	int depth = 0;
	QMap<QString, ItemPool*>::iterator pIt = m_pools.find(i_item_pool->getParentPath());
	if (pIt != m_pools.end())
		depth = (*pIt)->getDepth() + 1;
	i_item_pool->setDepth(depth);

	// Offset its renders (as pool item can be created after it render items):
	QMap<QString, QList<ItemRender*>>::iterator rIt = m_pool_renders.find(i_item_pool->getName());
	if (rIt != m_pool_renders.end())
		for (int i = 0; i < rIt.value().size(); i++)
			offsetHierarchy(rIt.value()[i]);
}

void ListRenders::offsetHierarchy(ItemRender * i_item_render)
{
	// Store render in "pool name" -> "renders items list" map:
	QMap<QString, QList<ItemRender*>>::iterator rIt = m_pool_renders.find(i_item_render->getPool());
	if (rIt == m_pool_renders.end())
		rIt = m_pool_renders.insert(i_item_render->getPool(), QList<ItemRender*>());
	if (false == rIt.value().contains(i_item_render))
		rIt.value().push_back(i_item_render);

	// Offset:
	int depth = 0;
	QMap<QString, ItemPool*>::iterator pIt = m_pools.find(i_item_render->getPool());
	if (pIt != m_pools.end())
		depth = (*pIt)->getDepth() + 1;
	i_item_render->setDepth(depth);
}

void ListRenders::removeRender(ItemRender * i_item_render)
{
	QMap<QString, QList<ItemRender*>>::iterator rIt = m_pool_renders.find(i_item_render->getPool());
	if (rIt == m_pool_renders.end())
	{
		AF_ERR << "ListRenders::removeRender: render['" << afqt::qtos(i_item_render->getName())
			<< "'] pool['" << afqt::qtos(i_item_render->getPool()) << "'] not found";
		return;
	}

	int count = rIt.value().removeAll(i_item_render);
	if (count != 1)
		AF_ERR << "ListRenders::removeRender: render['" << afqt::qtos(i_item_render->getName())
			<< "'] pool['" << afqt::qtos(i_item_render->getPool()) << "'] count = " << count;
}

void ListRenders::removePool(ItemPool * i_item_pool)
{
	int count = m_pools.remove(i_item_pool->getName());
	if (count != 1)
		AF_ERR << "ListRenders::removePool: pool['" << afqt::qtos(i_item_pool->getName())
			<< "'] m_pools count = " << count;

	count = m_pool_renders.remove(i_item_pool->getName());
	if (count > 1)
		AF_ERR << "ListRenders::removePool: pool['" << afqt::qtos(i_item_pool->getName())
			<< "'] m_pool_renders count = " << count;
}

void ListRenders::v_itemToBeDeleted(Item * i_item)
{
	switch(i_item->getType())
	{
	case Item::TRender:
		removeRender((ItemRender*)i_item);
		break;
	case Item::TPool:
		removePool((ItemPool*)i_item);
		break;
	default:
		AF_ERR << "ListRenders::v_itemToBeDeleted: Invalid item type.";
	}
}

void ListRenders::v_connectionLost()
{
	m_pool_renders.clear();
	m_pools.clear();
	ListNodes::v_connectionLost();
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

void ListRenders::actAddPool()
{
	Item * item = getCurrentItem();
	if (item == NULL)
		return;

	if (item->getType() != Item::TPool)
		return;

	bool ok;
	QString name = QInputDialog::getText(this, "Add Child Pool",
			QString("Enter a new \"%1\" child pool name").arg(item->getName()),
			QLineEdit::Normal, QString(), &ok);
	if (false == ok)
		return;

	displayInfo(QString("Adding a pool \"%1\" to \"%2\"").arg(name, item->getName()));

	addPool(item->getId(), name);
}

void ListRenders::addPool(int i_parent_id, const QString & i_child)
{
	std::ostringstream str;
	af::jsonActionOperationStart(str, "pools", "add_pool", "", std::vector<int>(1 , i_parent_id));
	str << ",\n\"name\":\"" << afqt::qtos(i_child) << "\"";
	af::jsonActionOperationFinish(str);
	Watch::sendMsg(af::jsonMsg(str));
}

void ListRenders::actRenderSetPool()
{
	Item * item = getCurrentItem();
	if (item == NULL)
		return;

	if (item->getType() != Item::TRender)
		return;

	bool ok;
	QString name = QInputDialog::getText(this, "Set Render Pool",
			"Enter render(s) new pool name",
			QLineEdit::Normal, QString(), &ok);
	if (false == ok)
		return;

	displayInfo(QString("Setting pool to \"%1\"").arg(name));

	renderSetPool(name);
}

void ListRenders::renderSetPool(const QString & i_name)
{
	Item::EType type = Item::TRender;
	std::vector<int> ids(getSelectedIds(type));
	std::ostringstream str;
	af::jsonActionOperationStart(str, "renders", "set_pool", "", ids);
	str << ",\n\"name\":\"" << afqt::qtos(i_name) << "\"";
	af::jsonActionOperationFinish(str);
	Watch::sendMsg(af::jsonMsg(str));
}

void ListRenders::actRenderReAssing()
{
	Item::EType type = Item::TRender;
	std::vector<int> ids(getSelectedIds(type));
	if (ids.size() == 0)
	{
		displayError("No renders selected.");
		return;
	}
	std::ostringstream str;
	af::jsonActionOperationStart(str, "renders", "reassign_pool", "", ids);
	af::jsonActionOperationFinish(str);
	Watch::sendMsg(af::jsonMsg(str));
}

void ListRenders::actNewRenderNimby()
{
	setParameter(Item::TPool, "new_nimby", "true");
}

void ListRenders::actNewRenderFree()
{
	setParameter(Item::TPool, "new_nimby", "false");
}

void ListRenders::actNewRenderPaused()
{
	setParameter(Item::TPool, "new_paused", "true");
}

void ListRenders::actNewRenderReady()
{
	setParameter(Item::TPool, "new_paused", "false");
}

void ListRenders::actCapacity()
{
	ItemRender* item = (ItemRender*)getCurrentItem();
	if( item == NULL ) return;
	int current = item->getCapacity();

	bool ok;
	int capacity = QInputDialog::getInt(this, "Change Capacity", "Enter New Capacity", current, -1, 1000000, 1, &ok);
	if( !ok) return;
	setParameter(Item::TRender, "capacity", af::itos(capacity));
}
void ListRenders::actMaxTasks()
{
	ItemRender* item = (ItemRender*)getCurrentItem();
	if( item == NULL ) return;
	int current = item->getMaxTasks();

	bool ok;
	int max_tasks = QInputDialog::getInt(this, "Change Maximum Tasksy", "Enter New Limit", current, -1, 1000000, 1, &ok);
	if( !ok) return;
	setParameter(Item::TRender, "max_tasks", af::itos(max_tasks));
}

void ListRenders::actNIMBY()       {setParameter(Item::TRender, "NIMBY",  "true" );}
void ListRenders::actNimby()       {setParameter(Item::TRender, "nimby",  "true" );}
void ListRenders::actFree()        {setParameter(Item::TRender, "nimby",  "false");}
void ListRenders::actSetPaused()   {setParameter(Item::TRender, "paused", "true" );}
void ListRenders::actUnsetPaused() {setParameter(Item::TRender, "paused", "false");}

void ListRenders::actUser()
{
	QString current = afqt::stoq( af::Environment::getUserName());

	bool ok;
	QString text = QInputDialog::getText(this, "Set User", "Enter User Name", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	setParameter(Item::TRender, "user_name", afqt::qtos( text));
}

void ListRenders::actEjectTasks()      { operation(Item::TRender, "eject_tasks"        ); }
void ListRenders::actEjectNotMyTasks() { operation(Item::TRender, "eject_tasks_keep_my"); }
void ListRenders::actExit()            { operation(Item::TRender, "exit"               ); }
void ListRenders::actReboot()          { operation(Item::TRender, "reboot"             ); }
void ListRenders::actShutdown()        { operation(Item::TRender, "shutdown"           ); }
void ListRenders::actWOLSleep()        { operation(Item::TRender, "wol_sleep"          ); }
void ListRenders::actWOLWake()         { operation(Item::TRender, "wol_wake"           ); }
void ListRenders::actDelete()          { operation(Item::TAny,    "delete"             ); }

void ListRenders::actRequestLog()      { getItemInfo(Item::TAny,    "log"      ); }
void ListRenders::actRequestTasksLog() { getItemInfo(Item::TRender, "tasks_log"); }
void ListRenders::actRequestInfo()     { getItemInfo(Item::TRender, "full"     ); }

void ListRenders::actRequestTaskInfo(int jid, int bnum, int tnum)
{
	WndTask::openTask( af::MCTaskPos( jid, bnum, tnum));
}

void ListRenders::slot_ServiceAdd()    {editServiceDialog("service_add",     "Add Service");    }
void ListRenders::actServiceRemove()   {editServiceDialog("service_remove",  "Remove Service"); }
void ListRenders::actServiceEnable()   {editServiceDialog("service_enable",  "Enable Service"); }
void ListRenders::slot_ServiceDisable(){editServiceDialog("service_disable", "Disable Service");}
void ListRenders::editServiceDialog(const QString & i_mode, const QString & i_dialog_caption)
{
	if (getCurrentItem() == NULL)
		return;

	QString dialog_tip("Enter service mask");
	if (i_mode == "service_add")
		dialog_tip = "Enter service name";

	bool ok;
	QString service_mask = QInputDialog::getText(this, i_dialog_caption, dialog_tip, QLineEdit::Normal, QString(), &ok);
	if (false == ok)
		return;

	if (service_mask.isEmpty())
	{
		displayError("Empty service name/mask");
		return;
	}

	std::string err;
	if (false == af::RegExp::Validate(afqt::qtos(service_mask), &err))
	{
		displayError(afqt::stoq(err));
		return;
	}

	slot_ServiceEdit(i_mode, service_mask);
}
void ListRenders::slot_ServiceEdit(QString i_mode, QString i_service)
{
	std::ostringstream str;
	Item::EType type = Item::TAny;
	std::vector<int> ids(getSelectedIds(type));
	af::jsonActionOperationStart(str, itemTypeToAf(type), "farm", "", ids);
	str << ",\n\"mode\":\"" << afqt::qtos(i_mode) << "\"";
	str << ",\n\"name\":\"" << afqt::qtos(i_service) << "\"";
	str << ",\n\"mask\":\"" << afqt::qtos(i_service) << "\"";
	af::jsonActionOperationFinish(str);
	Watch::sendMsg(af::jsonMsg(str));
}
void ListRenders::actClearServices()
{
	if (getCurrentItem() == NULL)
		return;

	std::ostringstream str;
	Item::EType type = Item::TAny;
	std::vector<int> ids(getSelectedIds(type));
	af::jsonActionOperationStart(str, itemTypeToAf(type), "farm", "", ids);
	str << ",\n\"mode\":\"clear_services\"";
	af::jsonActionOperationFinish(str);
	Watch::sendMsg(af::jsonMsg(str));
}

void ListRenders::slot_TicketPoolAdd(){ticketAdd(false);}
void ListRenders::slot_TicketHostAdd(){ticketAdd(true );}
void ListRenders::ticketAdd(bool i_host_ticket)
{
	bool ok;
	QString name = QInputDialog::getText(this,
			QString("Add %1 Ticket").arg(i_host_ticket ? "Host" : "Pool"),
			"Enter a new ticket name", QLineEdit::Normal, "", &ok);
	if (false == ok)
		return;

	ticketEdit(name, i_host_ticket);
}

void ListRenders::slot_TicketPoolEdit(const QString & i_name){ticketEdit(i_name, false);}
void ListRenders::slot_TicketHostEdit(const QString & i_name){ticketEdit(i_name, true );}
void ListRenders::ticketEdit(const QString & i_name, bool i_host_ticket)
{
	if (i_name.isEmpty())
	{
		displayError("Ticket name is empty.");
		return;
	}

	Item * item = getCurrentItem();
	if (item == NULL)
		return;
	ItemFarm * item_farm = (ItemFarm*)(item);

	int cur_count = 1;

	if (i_host_ticket)
	{
		QMap<QString, int>::const_iterator it = item_farm->m_tickets_host.find(i_name);
		if (it != item_farm->m_tickets_host.end())
			cur_count = it.value();
	}
	else
	{
		if (item->getType() == Item::TRender)
		{
			displayError("Render can have only host tickets.");
			return;
		}

		QMap<QString, int>::const_iterator it = item_farm->m_tickets_pool.find(i_name);
		if (it != item_farm->m_tickets_pool.end())
			cur_count = it.value();
	}

	bool ok;
	int new_count = QInputDialog::getInt(this,
			QString("Edit %1 Ticket").arg(i_host_ticket ? "Host" : "Pool"),
			QString("Enter %1 cur_count.\nType -1 to remove.").arg(i_name),
			cur_count, -1, 1<<30, 1, &ok);

	std::ostringstream str;
	Item::EType type = Item::TAny;
	std::vector<int> ids(getSelectedIds(type));
	af::jsonActionOperationStart(str, itemTypeToAf(type), "tickets", "", ids);
	str << ",\n\"name\":\"" << afqt::qtos(i_name) << "\"";
	str << ",\n\"count\":" << new_count;
	if (i_host_ticket)
		str << ",\n\"host\":true";
	af::jsonActionOperationFinish(str);
	Watch::sendMsg(af::jsonMsg(str));
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

	launchCmdStringExit(cmd, i_exit);
}
void ListRenders::actLaunchCmdString(    QString i_cmd) { launchCmdStringExit(i_cmd, false);}
void ListRenders::actLaunchCmdExitString(QString i_cmd) { launchCmdStringExit(i_cmd, true );}
void ListRenders::launchCmdStringExit(const QString & i_cmd, bool i_exit)
{
	std::ostringstream str;
	Item::EType type = Item::TRender;
	std::vector<int> ids(getSelectedIds(type));
	af::jsonActionOperationStart(str, "renders", "launch_cmd", "", ids);
	str << ",\n\"cmd\":\"" << afqt::qtos(i_cmd) << "\"";
	if (i_exit)
		str << ",\n\"exit\":true";
	af::jsonActionOperationFinish(str);
	Watch::sendMsg(af::jsonMsg(str));
}

