#include "listusers.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/monitor.h"
#include "../libafanasy/monitorevents.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "buttonpanel.h"
#include "buttonsmenu.h"
#include "ctrlsortfilter.h"
#include "itemuser.h"
#include "modelnodes.h"
#include "monitorhost.h"
#include "viewitems.h"
#include "watch.h"

#include <QtCore/QEvent>
#include <QtGui/QContextMenuEvent>
#include <QInputDialog>
#include <QLayout>
#include <QMenu>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

int     ListUsers::ms_SortType1      = CtrlSortFilter::TNUMRUNNINGTASKS;
int     ListUsers::ms_SortType2      = CtrlSortFilter::TNUMJOBS;
bool    ListUsers::ms_SortAscending1 = false;
bool    ListUsers::ms_SortAscending2 = false;
int     ListUsers::ms_FilterType     = CtrlSortFilter::TNAME;
bool    ListUsers::ms_FilterInclude  = true;
bool    ListUsers::ms_FilterMatch    = false;
std::string ListUsers::ms_FilterString = "";

ListUsers::ListUsers( QWidget* parent):
	ListNodes(  parent, "users")
{
	m_ctrl_sf = new CtrlSortFilter( this,
			&ms_SortType1, &ms_SortAscending1,
			&ms_SortType2, &ms_SortAscending2,
			&ms_FilterType, &ms_FilterInclude, &ms_FilterMatch, &ms_FilterString);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TNONE);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TPRIORITY);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TNAME);
	m_ctrl_sf->addSortType(   CtrlSortFilter::THOSTNAME);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TNUMJOBS);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TNUMRUNNINGTASKS);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TTIMEREGISTERED);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TTIMEACTIVITY);
	m_ctrl_sf->addFilterType( CtrlSortFilter::TNONE);
	m_ctrl_sf->addFilterType( CtrlSortFilter::TNAME);
	m_ctrl_sf->addFilterType( CtrlSortFilter::THOSTNAME);
	initSortFilterCtrl();


	// Add left panel buttons:
	ButtonPanel * bp; ButtonsMenu * bm;

	bp = addButtonPanel(Item::TUser, "LOG","users_log","Get user log.");
	connect(bp, SIGNAL(sigClicked()), this, SLOT(actRequestLog()));

	bm = addButtonsMenu(Item::TUser, "Solve","Choose jobs solving method.");
	bm->openMenu();

	bp = addButtonPanel(Item::TUser, "ORDER","users_solve_ordered","Solve jobs by order.");
	connect(bp, SIGNAL(sigClicked()), this, SLOT(actSolveJobsByOrder()));

	bp = addButtonPanel(Item::TUser, "PRIORITY","users_solve_priority","Solve jobs by priority.");
	connect(bp, SIGNAL(sigClicked()), this, SLOT(actSolveJobsByPriority()));

	resetButtonsMenu();

	bm = addButtonsMenu(Item::TUser, "Need","Choose jobs solving need.");
	bm->openMenu();

	bp = addButtonPanel(Item::TUser, "CAPACITY","users_solve_capacity","Solve jobs need by running tasks total capacity.");
	connect(bp, SIGNAL(sigClicked()), this, SLOT(actSolveJobsByCapacity()));

	bp = addButtonPanel(Item::TUser, "TASKS NUM","users_solve_tasksnum","Solve jobs need by running tasks number.");
	connect(bp, SIGNAL(sigClicked()), this, SLOT(actSolveJobsByTasksNum()));

	resetButtonsMenu();


	// Add parameters:
	if (af::Environment::VISOR() || (af::Environment::getPermUserModHisPriority()))
	{
		addParam_Num(Item::TUser, "priority", "Priority", "Priority number", 0, 250);
	}
	addParam_Str(Item::TUser, "annotation",                "Annotation",             "Annotation string");
	addParam_Num(Item::TUser, "max_running_tasks",         "Maximum Running",        "Maximum running tasks number", -1, 1<<20);
	addParam_Num(Item::TUser, "max_running_tasks_per_host","Max Run Per Host",       "Max run tasks on the same host", -1, 1<<20);
	addParam_REx(Item::TUser, "hosts_mask",                "Hosts Mask",             "Host names pattern that job can run on");
	addParam_REx(Item::TUser, "hosts_mask_exclude",        "Hosts Mask Exclude",     "Host names pattern that job will not run");
	addParam_Num(Item::TUser, "errors_avoid_host",         "Errors Job  Avoid Host", "Number of errors for job to avoid host", -1, 1<<10);
	addParam_Num(Item::TUser, "errors_task_same_host",     "Errors Task Avoid Host", "Number of errors for task to avoid host", -1, 1<<10);
	addParam_Num(Item::TUser, "errors_retries",            "Errors Retries",         "Number of errors task retries", -1, 1<<10);
	addParam_Hrs(Item::TUser, "errors_forgive_time",       "Errors Forgive Time",    "After this time host errors will be reset");
	addParam_Hrs(Item::TUser, "jobs_life_time",            "Jobs Life Time",         "After this time job will be deleted");


	m_parentWindow->setWindowTitle("Users");

	initListNodes();

	if( false == af::Environment::VISOR()) setAllowSelection( false);

	connect( (ModelNodes*)m_model, SIGNAL( nodeAdded( ItemNode *, const QModelIndex &)),
			                 this,   SLOT( userAdded( ItemNode *, const QModelIndex &)));
}

ListUsers::~ListUsers()
{
}

void ListUsers::contextMenuEvent(QContextMenuEvent *event)
{
	ItemUser* useritem = (ItemUser*)getCurrentItem();
	if( useritem == NULL ) return;
	bool me = false;
	if( useritem->getId() == MonitorHost::getUid()) me = true;


	QMenu menu(this);
	QMenu * submenu;
	QAction *action;

	action = new QAction( "Show Log", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actRequestLog() ));
	menu.addAction( action);

	if( me || af::Environment::VISOR() )
	{
		menu.addSeparator();
		submenu = new QMenu("Set Parameter", this);

		addMenuParameters(submenu);

		menu.addMenu( submenu);
		menu.addSeparator();

		submenu = new QMenu("Jobs Solving Method", this);

		action = new QAction("Method By Order", this);
		connect(action, SIGNAL(triggered()), this, SLOT(actSolveJobsByOrder()));
		submenu->addAction(action);

		action = new QAction("Method By Priority", this);
		connect(action, SIGNAL(triggered()), this, SLOT(actSolveJobsByPriority()));
		submenu->addAction(action);

		action = new QAction("Need By Capacity", this);
		connect(action, SIGNAL(triggered()), this, SLOT(actSolveJobsByCapacity()));
		submenu->addAction(action);

		action = new QAction("Need By Run Tasks", this);
		connect(action, SIGNAL(triggered()), this, SLOT(actSolveJobsByTasksNum()));
		submenu->addAction(action);

		menu.addMenu( submenu);

		menu.addSeparator();

		action = new QAction( "Custom Data", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actCustomData() ));
		menu.addAction( action);

		menu.addSeparator();

		action = new QAction( "Delete", this);
		action->setEnabled(useritem->jobs_num == 0);
		connect( action, SIGNAL( triggered() ), this, SLOT( actDelete() ));
		menu.addAction( action);
	}

	menu.exec( event->globalPos());
}

bool ListUsers::v_caseMessage( af::Msg * msg)
{
#ifdef AFOUTPUT
	msg->stdOut();
#endif
	switch( msg->type())
	{
	case af::Msg::TUsersList:
	{
		updateItems(msg, Item::TUser);
		calcTitle();
		subscribe();
		break;
	}
	default:
		return false;
	}
	return true;
}

bool ListUsers::v_processEvents( const af::MonitorEvents & i_me)
{
	if (i_me.m_events[af::Monitor::EVT_users_del].size())
	{
		deleteItems(i_me.m_events[af::Monitor::EVT_users_del], Item::TUser);
		calcTitle();
		return true;
	}

	std::vector<int> ids;

	for( int i = 0; i < i_me.m_events[af::Monitor::EVT_users_change].size(); i++)
		af::addUniqueToVect( ids, i_me.m_events[af::Monitor::EVT_users_change][i]);

	for( int i = 0; i < i_me.m_events[af::Monitor::EVT_users_add].size(); i++)
		af::addUniqueToVect( ids, i_me.m_events[af::Monitor::EVT_users_add][i]);

	if( ids.size())
	{
		get( ids);
		return true;
	}

	return false;
}

ItemNode* ListUsers::v_createNewItemNode(af::Node * i_afnode, Item::EType i_type, bool i_notify)
{
	return new ItemUser(this, (af::User*)i_afnode, m_ctrl_sf);
}

void ListUsers::userAdded(ItemNode * node, const QModelIndex & index)
{
	if (node->getId() == MonitorHost::getUid())
	{
		m_view->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
	}
}

void ListUsers::calcTitle()
{
	int total = count();
	int running = 0;
	for( int i = 0; i < total; i++)
	{
		ItemUser * itemuser = (ItemUser*)(m_model->item(i));
		if (itemuser->running_tasks_num > 0) running++;
	}
	m_parentWindow->setWindowTitle(QString("U[%1]: %2R").arg( total).arg( running));
}

void ListUsers::actDelete() { operation(Item::TUser, "delete"); }
void ListUsers::actSolveJobsByOrder()    {setParameter(Item::TUser, "solve_method", "\"solve_order\""   ); }
void ListUsers::actSolveJobsByPriority() {setParameter(Item::TUser, "solve_method", "\"solve_priority\""); }
void ListUsers::actSolveJobsByCapacity() {setParameter(Item::TUser, "solve_need",   "\"solve_capacity\""); }
void ListUsers::actSolveJobsByTasksNum() {setParameter(Item::TUser, "solve_need",   "\"solve_tasksnum\""); }

void ListUsers::actRequestLog() { getItemInfo(Item::TAny, "log"); }

