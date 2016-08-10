#include "listusers.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/monitor.h"
#include "../libafanasy/monitorevents.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "buttonpanel.h"
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
	ButtonPanel * bp;

	bp = addButtonPanel("LOG","users_log","Show user log.");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actRequestLog()));

	bp = addButtonPanel("PRI","users_priority","Set user priority.");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actPriority()));

	bp = addButtonPanel("FOR","users_errors_forgive_time","Set user errors forgive time.");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actErrorsForgiveTime()));

	bp = addButtonPanel("LIFE","users_jobs_life_time","Set jobs life time.");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actJobsLifeTime()));

	bp = addButtonPanel("ORD","users_solve_ordered","Solve jobs by order.");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actSolveJobsByOrder()));

	bp = addButtonPanel("PAR","users_solve_parallel","Solve jobs parallel.");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actSolveJobsParallel()));


	m_parentWindow->setWindowTitle("Users");

	init();

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

		action = new QAction( "Annonate", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actAnnotate() ));
		menu.addAction( action);

		menu.addSeparator();

		action = new QAction( "Set Max Running Tasks", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actMaxRunningTasks() ));
		menu.addAction( action);
		action = new QAction( "Set Hosts Mask", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actHostsMask() ));
		menu.addAction( action);
		action = new QAction( "Set Exclude Hosts Mask", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actHostsMaskExclude() ));
		menu.addAction( action);

		if(( af::Environment::VISOR()) || ( af::Environment::getPermUserModHisPriority()))
		{
			action = new QAction( "Set Prority", this);
			connect( action, SIGNAL( triggered() ), this, SLOT( actPriority() ));
			menu.addAction( action);
		}

		menu.addSeparator();

		action = new QAction( "Set Job Avoid Errors", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actErrorsAvoidHost() ));
		menu.addAction( action);
		action = new QAction( "Set Task Avoid Errors", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actErrorsSameHost() ));
		menu.addAction( action);
		action = new QAction( "Set Task Error Retries", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actErrorRetries() ));
		menu.addAction( action);
		action = new QAction( "Set Errors Forgive Time", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actErrorsForgiveTime() ));
		menu.addAction( action);

		menu.addSeparator();

		action = new QAction( "Set Jobs Life Time", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actJobsLifeTime() ));
		menu.addAction( action);

		menu.addSeparator();

		submenu = new QMenu("Jobs Solving Method", this);

		action = new QAction("By Order", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actSolveJobsByOrder() ));
		submenu->addAction( action);

		action = new QAction("Parallel", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actSolveJobsParallel() ));
		submenu->addAction( action);

		menu.addMenu( submenu);

		menu.addSeparator();

		action = new QAction( "Custom Data", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actCustomData() ));
		menu.addAction( action);

		menu.addSeparator();

		action = new QAction( "Delete", this);
		action->setEnabled( useritem->numjobs == 0 );
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
		updateItems( msg);
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
	if( i_me.m_events[af::Monitor::EVT_users_del].size())
	{
		deleteItems( i_me.m_events[af::Monitor::EVT_users_del]);
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

ItemNode* ListUsers::v_createNewItem( af::Node * i_node, bool i_subscibed)
{
	return new ItemUser( (af::User*)i_node, m_ctrl_sf);
}

void ListUsers::userAdded( ItemNode * node, const QModelIndex & index)
{
//printf("node->getId()=%d ,   Watch::getUid()=%d,  row=%d\n", node->getId(), Watch::getUid(), index.row());
	if( node->getId() == MonitorHost::getUid()) m_view->selectionModel()->select( index, QItemSelectionModel::Select);
}

void ListUsers::calcTitle()
{
	int total = count();
	int running = 0;
	for( int i = 0; i < total; i++)
	{
		ItemUser * itemuser = (ItemUser*)(m_model->item(i));
		if( itemuser->numrunningtasks > 0 ) running++;
	}
	m_parentWindow->setWindowTitle(QString("U[%1]: %2R").arg( total).arg( running));
}

void ListUsers::actErrorsAvoidHost()
{
	ItemUser* useritem = (ItemUser*)getCurrentItem();
	if( useritem == NULL ) return;
	int current = useritem->errors_avoidhost;

	bool ok;
	int value = QInputDialog::getInt(this, "Errors to avoid host", "Enter Number of Errors", current, 0, 99, 1, &ok);
	if( !ok) return;

	setParameter("errors_avoid_host", value);
}

void ListUsers::actErrorsSameHost()
{
	ItemUser* useritem = (ItemUser*)getCurrentItem();
	if( useritem == NULL ) return;
	int current = useritem->errors_tasksamehost;

	bool ok;
	int value = QInputDialog::getInt(this, "Errors same host", "Enter Number of Errors", current, 0, 99, 1, &ok);
	if( !ok) return;

	setParameter("errors_task_same_host", value);
}

void ListUsers::actErrorRetries()
{
	ItemUser* useritem = (ItemUser*)getCurrentItem();
	if( useritem == NULL ) return;
	int current = useritem->errors_retries;

	bool ok;
	int value = QInputDialog::getInt(this, "Auto retry error tasks", "Enter Number of Errors", current, 0, 99, 1, &ok);
	if( !ok) return;

	setParameter("errors_retries", value);
}

void ListUsers::actErrorsForgiveTime()
{
	ItemUser* useritem = (ItemUser*)getCurrentItem();
	if( useritem == NULL ) return;
	double cur = double( useritem->errors_forgivetime ) / (60.0*60.0);

	bool ok;
	double hours = QInputDialog::getDouble( this, "Errors Forgive Time", "Enter number of hours (0=infinite)", cur, 0, 365*24, 3, &ok);
	if( !ok) return;

	setParameter("errors_forgive_time", int( hours * 60.0 * 60.0 ));
}

void ListUsers::actJobsLifeTime()
{
	ItemUser* useritem = (ItemUser*)getCurrentItem();
	if( useritem == NULL ) return;
	double cur = double( useritem->jobs_lifetime ) / (60.0*60.0);

	bool ok;
	double hours = QInputDialog::getDouble( this, "Jobs Life Time", "Enter number of hours (0=infinite)", cur, 0, 365*24, 3, &ok);
	if( !ok) return;

	setParameter("jobs_life_time", int( hours * 60.0 * 60.0 ));
}

void ListUsers::actMaxRunningTasks()
{
	ItemUser* useritem = (ItemUser*)getCurrentItem();
	if( useritem == NULL ) return;
	int current = useritem->maxrunningtasks;

	bool ok;
	int max = QInputDialog::getInt(this, "Change Maximum Running Tasks", "Enter Number", current, -1, 9999, 1, &ok);
	if( !ok) return;

	setParameter("max_running_tasks", max);
}

void ListUsers::actHostsMask()
{
	ItemUser* useritem = (ItemUser*)getCurrentItem();
	if( useritem == NULL ) return;
	QString current = useritem->hostsmask;

	bool ok;
	QString mask = QInputDialog::getText(this, "Change Hosts Mask", "Enter Mask", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	setParameterRE("hosts_mask", afqt::qtos( mask));
}

void ListUsers::actHostsMaskExclude()
{
	ItemUser* useritem = (ItemUser*)getCurrentItem();
	if( useritem == NULL ) return;
	QString current = useritem->hostsmask_exclude;

	bool ok;
	QString mask = QInputDialog::getText(this, "Change Exclude Mask", "Enter Mask", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	setParameterRE("hosts_mask_exclude", afqt::qtos( mask));
}

void ListUsers::actDelete() { operation("delete"); }
void ListUsers::actSolveJobsByOrder()  { setParameter("solve_parallel", "false", false); }
void ListUsers::actSolveJobsParallel() { setParameter("solve_parallel", "true",  false); }

void ListUsers::actRequestLog() { getItemInfo("log"); }

