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

	bp = addButtonPanel(Item::TUser, "PAUSE","user_pause","Pause selected users.","P");
	connect(bp, SIGNAL(sigClicked()), this, SLOT(actSetPaused()));

	bp = addButtonPanel(Item::TUser, "START","user_unpause","Start (Unpause) selected users.","S");
	connect(bp, SIGNAL(sigClicked()), this, SLOT(actUnsetPaused()));

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

	bp = addButtonPanel(Item::TAny, "CUSTOM DATA","node_custom_data","Edit node custom data.");
	connect(bp, SIGNAL(sigClicked()), this, SLOT(actCustomData()));

	if (af::Environment::GOD())
	{
		bp = addButtonPanel(Item::TUser, "DELETE","users_solve_tasksnum",
				"Solve jobs need by running tasks number.",
				"" /*hotkey*/, true /*double-click*/);
		connect(bp, SIGNAL(sigClicked()), this, SLOT(actDelete()));
	}


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


	this->setWindowTitleWithPrefix("Users");

	initListNodes();

	if (false == af::Environment::VISOR())
		setAllowSelection(false);

	connect((ModelNodes*)m_model, SIGNAL(nodeAdded(ItemNode *, const QModelIndex &)),
			                 this,  SLOT(userAdded(ItemNode *, const QModelIndex &)));
}

ListUsers::~ListUsers()
{
}

void ListUsers::contextMenuEvent(QContextMenuEvent *event)
{
    return;
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
	for (int i = 0; i < total; i++)
	{
		ItemUser * itemuser = static_cast<ItemUser*>(m_model->item(i));
		if (itemuser->running_tasks_num > 0) running++;
	}
	this->setWindowTitleWithPrefix(QString("Users: %1, Running %2").arg(total).arg(running));
}

void ListUsers::actDelete() { operation(Item::TUser, "delete"); }
void ListUsers::actSetPaused()   {setParameter(Item::TUser, "paused", "true" );}
void ListUsers::actUnsetPaused() {setParameter(Item::TUser, "paused", "false");}
void ListUsers::actSolveJobsByOrder()    {setParameterStr(Item::TUser, "solve_method", "solve_order"   ); }
void ListUsers::actSolveJobsByPriority() {setParameterStr(Item::TUser, "solve_method", "solve_priority"); }
void ListUsers::actSolveJobsByCapacity() {setParameterStr(Item::TUser, "solve_need",   "solve_capacity"); }
void ListUsers::actSolveJobsByTasksNum() {setParameterStr(Item::TUser, "solve_need",   "solve_tasksnum"); }

void ListUsers::actRequestLog() { getItemInfo(Item::TAny, "log"); }

