#include "listjobs.h"

#include "../libafanasy/address.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mctaskup.h"
#include "../libafanasy/service.h"

#include "buttonpanel.h"
#include "itemjob.h"
#include "ctrljobs.h"
#include "ctrlsortfilter.h"
#include "monitorhost.h"
#include "modelnodes.h"
#include "paramspaneljob.h"
#include "viewitems.h"
#include "watch.h"

#include <QtCore/QDateTime>
#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtGui/QContextMenuEvent>
#include <QBoxLayout>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QSplitter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

int     ListJobs::ms_SortType1      = CtrlSortFilter::TNONE;
int     ListJobs::ms_SortType2      = CtrlSortFilter::TNONE;
bool    ListJobs::ms_SortAscending1 = false;
bool    ListJobs::ms_SortAscending2 = false;
int     ListJobs::ms_FilterType     = CtrlSortFilter::TNAME;
bool    ListJobs::ms_FilterInclude  = true;
bool    ListJobs::ms_FilterMatch    = false;
std::string ListJobs::ms_FilterString = "";

int     ListJobs::ms_SortType1_SU      = CtrlSortFilter::TTIMECREATION;
int     ListJobs::ms_SortType2_SU      = CtrlSortFilter::TTIMERUN;
bool    ListJobs::ms_SortAscending1_SU = false;
bool    ListJobs::ms_SortAscending2_SU = false;
int     ListJobs::ms_FilterType_SU     = CtrlSortFilter::TUSERNAME;
bool    ListJobs::ms_FilterInclude_SU  = true;
bool    ListJobs::ms_FilterMatch_SU    = false;
std::string ListJobs::ms_FilterString_SU = "";

uint32_t ListJobs::ms_hide_flags = e_HideHidden;

ListJobs::ListJobs(QWidget * i_parent, bool i_listwork, const std::string & i_name):
	ListNodes(i_parent, i_name),
	m_listwork(i_listwork),
	m_all_blocks_menu_shown(false)
{
	if( af::Environment::VISOR() || m_listwork)
		m_ctrl_sf = new CtrlSortFilter( this,
			&ms_SortType1_SU, &ms_SortAscending1_SU,
			&ms_SortType2_SU, &ms_SortAscending2_SU,
			&ms_FilterType_SU, &ms_FilterInclude_SU, &ms_FilterMatch_SU, &ms_FilterString_SU);
	else
		m_ctrl_sf = new CtrlSortFilter( this,
			&ms_SortType1, &ms_SortAscending1,
			&ms_SortType2, &ms_SortAscending2,
			&ms_FilterType, &ms_FilterInclude, &ms_FilterMatch, &ms_FilterString);

	m_ctrl_sf->addSortType(   CtrlSortFilter::TNONE);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TTIMECREATION);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TTIMERUN);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TTIMESTARTED);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TTIMEFINISHED);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TNUMRUNNINGTASKS);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TSERVICE);
	m_ctrl_sf->addSortType(   CtrlSortFilter::TNAME);
	m_ctrl_sf->addFilterType( CtrlSortFilter::TNONE);
	m_ctrl_sf->addFilterType( CtrlSortFilter::TNAME);
	m_ctrl_sf->addFilterType( CtrlSortFilter::TSERVICE);
	if( af::Environment::VISOR())
	{
		m_ctrl_sf->addSortType(   CtrlSortFilter::TPRIORITY);
		m_ctrl_sf->addSortType(   CtrlSortFilter::THOSTNAME);
		m_ctrl_sf->addSortType(   CtrlSortFilter::TUSERNAME);
		m_ctrl_sf->addFilterType( CtrlSortFilter::THOSTNAME);
		m_ctrl_sf->addFilterType( CtrlSortFilter::TUSERNAME);
	}

	// Get stored hide flags:
	m_hide_flags = ms_hide_flags;

	initSortFilterCtrl();

	CtrlJobs * control = new CtrlJobs(m_ctrl_sf, this, m_listwork);
	m_toplayout->addWidget(control);

	// Add left panel buttons:
	if (af::Environment::VISOR() || (false == m_listwork))
	{
	ButtonPanel * bp;

	bp = addButtonPanel(Item::TAny, "LOG","jobs_log","Show job log.");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actRequestLog()));

	bp = addButtonPanel(Item::TAny, "PAUSE","jobs_pause","Pause selected jobs or branches.","P");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actPause()));

	bp = addButtonPanel(Item::TAny, "START","jobs_start","Start selected jobs or branches.","S");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actStart()));

	bp = addButtonPanel(Item::TJob, "STOP","jobs_stop","Stop selected jobs tasks and pause jobs.","", true);
	connect(bp, SIGNAL(sigClicked()), this, SLOT(actStop()));

	if (false == af::Environment::VISOR())
	{
		addButtonsMenu(Item::TJob, "Move","Move job(s).");

		bp = addButtonPanel(Item::TJob, "Up", "move_up", "Move job(s) up.","u");
		connect(bp, SIGNAL(sigClicked()), this, SLOT(actMoveUp()));

		bp = addButtonPanel(Item::TJob, "Down", "move_down", "Move job(s) down.","d");
		connect(bp, SIGNAL(sigClicked()), this, SLOT(actMoveDown()));

		bp = addButtonPanel(Item::TJob, "Top", "move_top", "Move job(s) top.","t");
		connect(bp, SIGNAL(sigClicked()), this, SLOT(actMoveTop()));

		bp = addButtonPanel(Item::TJob, "Bottom", "move_bottom", "Move job(s) bottom.","b");
		connect(bp, SIGNAL(sigClicked()), this, SLOT(actMoveBottom()));

		resetButtonsMenu();
	}

	addButtonsMenu(Item::TJob, "Restart","Restart jobs tasks menu.");

	bp = addButtonPanel(Item::TJob, "ALL TASKS","jobs_restart","Restart all tasks of all selected jobs.","", true);
	connect(bp, SIGNAL(sigClicked()), this, SLOT(actRestart()));

	bp = addButtonPanel(Item::TJob, "AND PAUSE","jobs_restart_pause","Restart tasks and pause selected jobs.","", true);
	connect(bp, SIGNAL(sigClicked()), this, SLOT(actRestartPause()));

	bp = addButtonPanel(Item::TJob, "WARNINGS","jobs_restart_warning","Restart tasks with warnings.","", true);
	connect(bp, SIGNAL(sigClicked()), this, SLOT(actRestartWarnings()));

	bp = addButtonPanel(Item::TJob, "RUNNING","jobs_restart_running","Restart runnning tasks.","", true);
	connect(bp, SIGNAL(sigClicked()), this, SLOT(actRestartRunning()));

	bp = addButtonPanel(Item::TJob, "SKIPPED","jobs_restart_skipped","Restart skipped tasks.","", true);
	connect(bp, SIGNAL(sigClicked()), this, SLOT(actRestartSkipped()));

	bp = addButtonPanel(Item::TJob, "DONE","jobs_restart_done","Restart done tasks.","", true);
	connect(bp, SIGNAL(sigClicked()), this, SLOT(actRestartDone()));

	resetButtonsMenu();

	addButtonsMenu(Item::TJob, "Errors","Show/Manipulate error hosts and tasks.");

	bp = addButtonPanel(Item::TJob, "SHOW HOSTS","jobs_show_err_hosts","Show error hosts.");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actRequestErrorHostsList()));

	bp = addButtonPanel(Item::TJob, "RESET HOSTS","jobs_reset_avoid_hosts","Reset error hosts.","E");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actResetErrorHosts()));

	bp = addButtonPanel(Item::TJob, "RETRY TASKS","jobs_restart_error_tasks","Restart error tasks.","R");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actRestartErrors()));

	resetButtonsMenu();

	bp = addButtonPanel(Item::TJob, "LISTEN","jobs_listen","Listen job running tasks ouput.");
	connect(bp, SIGNAL(sigClicked()), this, SLOT(actListenJob()));

	bp = addButtonPanel(Item::TAny, "CUSTOM DATA","node_custom_data","Edit node custom data.");
	connect(bp, SIGNAL(sigClicked()), this, SLOT(actCustomData()));

	if (m_listwork)
	{
		bp = addButtonPanel(Item::TAny, "DELETE","work_delete","Delete selected jobs or branches.","", true);
		connect(bp, SIGNAL(sigClicked()), this, SLOT(actDelete()));
	}
	else
	{
		bp = addButtonPanel(Item::TAny, "DELETE","jobs_delete","Delete selected jobs.","", true);
		connect(bp, SIGNAL(sigClicked()), this, SLOT(actDelete()));
	}

	if (false == af::Environment::VISOR() && (false == m_listwork))
	{
		bp = addButtonPanel(Item::TJob, "DEL DONE","jobs_delete_done","Delete all done jobs.","",
				true /*double click*/, true /*always active*/);
		connect(bp, SIGNAL(sigClicked()), this, SLOT(actDeleteDone()));
	}
	}

	// Add parameters
	if ((af::Environment::VISOR()) || (af::Environment::getPermUserModJobPriority()))
	{
		int max = 200;
		if(( isTypeUsers()) && ( true != af::Environment::VISOR()))
			max = af::Environment::getPriority();
		addParam_Num(Item::TAny, "priority",              "Priority",           "Priority number", 0, max);
	}
	addParam_Str(Item::TAny, "annotation",                "Annotation",         "Annotation string");
	addParam_separator();
	addParam_MSI(Item::TJob, "pools",                     "Pools",              "Pools priorities");
	addParam_separator();
	addParam_Tim(Item::TJob, "time_wait",                 "Wait Time",          "Time to wait to start");
	addParam_separator();
	addParam_Num(Item::TAny, "max_running_tasks",         "Maximum Running",    "Maximum running tasks number", -1, 1<<20);
	addParam_Num(Item::TAny, "max_running_tasks_per_host","Max Run Per Host",   "Max run tasks on the same host", -1, 1<<20);
	addParam_separator();
	addParam_REx(Item::TAny, "hosts_mask",                "Hosts Mask",         "Host names pattern that job can run on");
	addParam_REx(Item::TAny, "hosts_mask_exclude",        "Hosts Mask Exclude", "Host names pattern that job will not run");
	addParam_REx(Item::TJob, "depend_mask",               "Depend Mask",        "Jobs name mask to wait");
	addParam_REx(Item::TJob, "depend_mask_global",        "Global Depend",      "Depend mask for jobs from any user");
	addParam_separator();
	addParam_Num(Item::TBranch, "max_tasks_per_second",   "Max Tasks Per Second", "Maximum tasks starts per second", -1, 1<<20);
	addParam_Hrs(Item::TJob,    "time_life",              "Life Time",            "Time to be deleted after creation");
	addParam_separator();
	addParam_Num(Item::TJob, "need_power",      "Power Needed",       "Job need client that has at least this power", -1, 1<<30);
	addParam_REx(Item::TJob, "need_os",         "OS Needed",          "Job will run only on this OS");
	addParam_MiB(Item::TJob, "need_memory",     "Memory Needed",      "Job need client to have at least this free memory (GB)", -1, 1<<30);
	addParam_GiB(Item::TJob, "need_hdd",        "HDD Needed",         "Job need client to have at least this free HDD space (GB)", -1, 1<<30);
	addParam_REx(Item::TJob, "need_properties", "Properties Needed",  "Job need client that has such properties");

	m_paramspanel = new ParamsPanelJob();
	initListNodes();

	QTimer * timer = new QTimer(this);
	timer->start( 1000 * af::Environment::getWatchRefreshGuiSec());
	connect( timer, SIGNAL( timeout()), this, SLOT( repaintItems()));

	this->setWindowTitleWithPrefix("Jobs:");
}

ListJobs::~ListJobs()
{
}

void ListJobs::v_hideChanged()
{
	// Store hide flags:
	ms_hide_flags = m_hide_flags;
}

void ListJobs::v_showFunc()
{
	if (Watch::isConnected() == false) return;

	if (af::Environment::VISOR() || m_listwork)
		get();
	else
	{
		if( MonitorHost::getUid() > 0 )
		{
			std::string str = "\"type\":\"jobs\",\"uids\":[";
			str += af::itos( MonitorHost::getUid()) + "]";
			Watch::get( str);
		}
		else
			if( m_parentWindow != (QWidget*)Watch::getDialog()) close();
	}
}

void ListJobs::contextMenuEvent( QContextMenuEvent *event)
{
	QMenu menu(this);
	QMenu * submenu;
	QAction *action;

	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	int selectedItemsCount = getSelectedItemsCount();

	if( jobitem->folders.size())
	{
		if( af::Environment::hasRULES())
		{
			action = new QAction("Open RULES", this);
			connect( action, SIGNAL( triggered() ), this, SLOT( actOpenRULES() ));
			menu.addAction( action);

			menu.addSeparator();
		}

		submenu = new QMenu("Folders", this);

		QMapIterator<QString,QString> it( jobitem->folders);
		while( it.hasNext())
		{
			it.next();

			action = new QAction( QString("\"") + it.key() + "\":", this);
			action->setEnabled( false);
			submenu->addAction( action);

			ActionString * action_str = new ActionString( it.value(), it.value(), this);
			connect( action_str, SIGNAL( triggeredString(QString) ), this, SLOT( actBrowseFolder(QString) ));
			submenu->addAction( action_str);
		}

		menu.addMenu( submenu);
		menu.addSeparator();
	}

	action = new QAction( "Show Log", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actRequestLog() ));
	menu.addAction( action);

	if (jobitem->has_tasks_trying_next)
	{
		action = new QAction("Reset Trying Tasks", this);
		connect(action, SIGNAL(triggered()), this, SLOT(actResetTryingNextTasks()));
		menu.addAction(action);
	}

	action = new QAction( "Show Error Hosts", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actRequestErrorHostsList() ));
	menu.addAction( action);

	action = new QAction( "Reset Error Hosts", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actResetErrorHosts() ));
	menu.addAction( action);

	action = new QAction( "Restart Error Tasks", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actRestartErrors() ));
	if( selectedItemsCount == 1) action->setEnabled( jobitem->state & AFJOB::STATE_ERROR_MASK);
	menu.addAction( action);

	menu.addSeparator();

	submenu = new QMenu("Start/Stop/Restart", this);
	menu.addMenu( submenu);

	action = new QAction("Start Job", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actStart()   ));
	if( selectedItemsCount == 1) action->setEnabled( jobitem->state & AFJOB::STATE_OFFLINE_MASK);
	submenu->addAction( action);

	action = new QAction("Pause Job", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actPause()   ));
	if( selectedItemsCount == 1) action->setEnabled( false == (jobitem->state & AFJOB::STATE_OFFLINE_MASK));
	submenu->addAction( action);

	action = new QAction("Stop Job", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actStop()    ));
	if( selectedItemsCount == 1) action->setEnabled( jobitem->state & AFJOB::STATE_RUNNING_MASK);
	submenu->addAction( action);

	action = new QAction("Restart Job", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actRestart() ));
	if( selectedItemsCount == 1) action->setEnabled(( jobitem->time_started != 0 ) || ( jobitem->state & AFJOB::STATE_SKIPPED_MASK ));
	submenu->addAction( action);

	action = new QAction("Restart&&Pause", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actRestartPause() ));
	if( selectedItemsCount == 1) action->setEnabled(( jobitem->time_started != 0 ) || ( jobitem->state & AFJOB::STATE_SKIPPED_MASK ));
	submenu->addAction( action);

	submenu->addSeparator();

	action = new QAction( "Restart Warning Tasks", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actRestartWarnings() ));
	if( selectedItemsCount == 1) action->setEnabled( jobitem->state & AFJOB::STATE_WARNING_MASK);
	submenu->addAction( action);

	action = new QAction( "Restart Running Tasks", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actRestartRunning() ));
	if( selectedItemsCount == 1) action->setEnabled( jobitem->state & AFJOB::STATE_RUNNING_MASK);
	submenu->addAction( action);

	action = new QAction( "Restart Skipped Tasks", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actRestartSkipped() ));
	if( selectedItemsCount == 1) action->setEnabled( jobitem->state & AFJOB::STATE_SKIPPED_MASK);
	submenu->addAction( action);

	action = new QAction( "Restart Done Tasks", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actRestartDone() ));
	submenu->addAction( action);

	menu.addSeparator();

	if( af::Environment::VISOR() == false)
	{
		  action = new QAction( "Move Up", this);
		  connect( action, SIGNAL( triggered() ), this, SLOT( actMoveUp() ));
		  menu.addAction( action);
		  action = new QAction( "Move Down", this);
		  connect( action, SIGNAL( triggered() ), this, SLOT( actMoveDown() ));
		  menu.addAction( action);
		  action = new QAction( "Move Top", this);
		  connect( action, SIGNAL( triggered() ), this, SLOT( actMoveTop() ));
		  menu.addAction( action);
		  action = new QAction( "Move Bottom", this);
		  connect( action, SIGNAL( triggered() ), this, SLOT( actMoveBottom() ));
		  menu.addAction( action);
	}
	else
	{
		action = new QAction("Change Owner", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actSetUser() ));
		menu.addAction( action);
		action = new QAction("Change Branch", this);
		connect(action, SIGNAL(triggered()), this, SLOT(actChangeBranch()));
		menu.addAction(action);
	}
	menu.addSeparator();
	
	submenu = new QMenu( "Annotation", this);
	
	const std::vector<std::string> annotations = af::Environment::getAnnotations();
	for( int p = 0; p < annotations.size(); p++)
	{
		QString annotation = afqt::stoq( annotations[p]);
		ActionString * action_str = new ActionString(annotation, annotation, this);
		connect( action_str, SIGNAL( triggeredString(QString) ), this, SLOT( actAnnotate(QString) ));
		submenu->addAction( action_str);
	}
	action = new QAction( "Custom", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actAnnotate() ));
	submenu->addAction( action);
	
	menu.addMenu( submenu);
	
	menu.addSeparator();

	submenu = new QMenu( "Set Parameter", this);

	addMenuParameters(submenu);

	submenu->addSeparator();

	action = new QAction( "Preview Approval", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actPreviewApproval() ));
	submenu->addAction( action);
	action = new QAction( "No Preview Approval", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actNoPreviewApproval() ));
	submenu->addAction( action);

	submenu->addSeparator();

	action = new QAction( "Hidden", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actSetHidden() ));
	submenu->addAction( action);
	action = new QAction( "Not Hidden", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actUnsetHidden() ));
	submenu->addAction( action);

	submenu->addSeparator();

	action = new QAction( "Post Command", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actPostCommand() ));
	submenu->addAction( action);

	menu.addMenu( submenu);

	menu.addSeparator();

	m_all_blocks_menu_shown = false;
	if( selectedItemsCount == 1)
	{
		if( jobitem->getBlocksNum() > 1)
		{
			submenu = new QMenu( "Tasks Blocks", this);
			for (int b = -1; b < jobitem->getBlocksNum(); b++)
			{
				QMenu * subsubmenu = new QMenu(b == -1 ? QString("_to_all_") : jobitem->getBlockName(b), this);
				if (b == -1)
				{
					// This menu changes all job blocks
					connect(subsubmenu, SIGNAL(aboutToShow()), this, SLOT(slot_BlocksMenuForAll()));
				}
				else
				{
					connect(subsubmenu, SIGNAL(aboutToShow()), this, SLOT(slot_BlocksMenuNotAll()));
				}
				jobitem->getBlockInfo(b==-1?0:b)->generateMenu(subsubmenu);
				submenu->addMenu(subsubmenu);
			}
		}
		else
		{
			submenu = new QMenu("Tasks Block", this);
			jobitem->getBlockInfo(0)->generateMenu(submenu);
			connect(submenu, SIGNAL(aboutToShow()), this, SLOT(slot_BlocksMenuNotAll()));
		}
	}
	else
	{
		submenu = new QMenu("All Selected Blocks", this);
		jobitem->getBlockInfo(0)->generateMenu( submenu);
		connect(submenu, SIGNAL(aboutToShow()), this, SLOT(slot_BlocksMenuForAll()));
	}
	menu.addMenu( submenu);

	menu.addSeparator();

	action = new QAction( "Listen", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actListenJob() ));
	menu.addAction( action);

	menu.addSeparator();

	// System job ID is 1, and can not be deleted
	if( jobitem->getId() != 1 )
	{
		submenu = new QMenu( "Delete", this);

		action = new QAction( "Delete Selected", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actDelete()));
		submenu->addAction( action);

		if (false == af::Environment::VISOR())
		{
			action = new QAction("Delete All Done", this);
			connect(action, SIGNAL(triggered()), this, SLOT(actDeleteDone()));
			submenu->addAction(action);
		}

		menu.addMenu( submenu);
	}

	menu.exec(event->globalPos());
}
void ListJobs::slot_BlocksMenuForAll() {m_all_blocks_menu_shown = true; }
void ListJobs::slot_BlocksMenuNotAll() {m_all_blocks_menu_shown = false;}

bool ListJobs::v_caseMessage( af::Msg * msg)
{
	switch (msg->type())
	{
	case af::Msg::TJobsList:
	{
		if (updateItems(msg, Item::TJob))
		{
			// We should get user jobs order only if:
			// user is not admin, it is not list work (it inherits this class) and sorting is disabled
			if ((af::Environment::VISOR() == false) && (false == m_listwork) && (m_ctrl_sf->isSortDisabled()))
				getUserJobsOrder();
		}

		if (false == isSubscribed())
		{
			if (af::Environment::VISOR() == false)
			{
				m_view->scrollToBottom();
			}
			subscribe();
		}

		calcTotals();

		break;
	}
	case af::Msg::TUserJobsOrder:
	{
		if ((af::Environment::VISOR() == false) && (false == m_listwork) && (m_ctrl_sf->isSortDisabled()))
		{
			af::MCGeneral ids(msg);
			AF_DEBUG << "Jobs order received: " << ids.v_generateInfoString(true);
			if (ids.getId() == MonitorHost::getUid())
				sortMatch(ids.getList());
		}
		break;
	}

	default:
		return false;
	}

	return true;
}

bool ListJobs::v_processEvents( const af::MonitorEvents & i_me)
{
	bool processed = false;

	if (i_me.m_events[af::Monitor::EVT_jobs_del].size())
	{
		deleteItems(i_me.m_events[af::Monitor::EVT_jobs_del], Item::TJob);
		calcTotals();
		processed = true;
	}

	std::vector<int> ids;

	for( int i = 0; i < i_me.m_events[af::Monitor::EVT_jobs_change].size(); i++)
		af::addUniqueToVect( ids, i_me.m_events[af::Monitor::EVT_jobs_change][i]);

	for( int i = 0; i < i_me.m_events[af::Monitor::EVT_jobs_add].size(); i++)
		af::addUniqueToVect( ids, i_me.m_events[af::Monitor::EVT_jobs_add][i]);

	if( ids.size())
	{
		get( ids);
		processed = true;
	}

	if( i_me.m_jobs_order_ids.size())
	{
		if ((af::Environment::VISOR() == false) && (false == m_listwork) && (m_ctrl_sf->isSortDisabled()))
		{
			sortMatch( i_me.m_jobs_order_ids);
		}

		processed = true;
	}

	return processed;
}

ItemNode * ListJobs::v_createNewItemNode(af::Node * i_afnode, Item::EType i_type, bool i_notify)
{
	af::Job * job = static_cast<af::Job*>(i_afnode);

	if (false == af::Environment::VISOR())
	{
		// User should not see ther users jobs if watch is not in admin mode.
		// But just switching from admin mode, prevous asked jobs can arrive.
		if (job->getUserName() != af::Environment::getUserName())
			return NULL;
	}

	return new ItemJob(this, false /*in work list*/, job, m_ctrl_sf, i_notify);
}

void ListJobs::v_itemToBeDeleted(Item * i_item)
{
	// If there is not more items we can delete all collapsed jobs serials.
	// On a job deletion we remove its serial, but deletion can happen by other GUI when this GUI is closed.
	// So w/o clearing collapsed jobs serials list can grow unlimited with time.
	if (count() == 0)
		afqt::QEnvironment::clearCollapsedJobSerials();
}

void ListJobs::v_resetSorting()
{
	if ((af::Environment::VISOR() == false) && (m_listwork == false))
		getUserJobsOrder();
}


void ListJobs::jobsHeightRecalculate()
{
	for( int i = 0; i < m_model->count(); i++)
	{
		Item * item = m_model->item(i);
		if (item->getType() != Item::TJob)
			continue;
		ItemJob * job = static_cast<ItemJob*>(item);
		job->resizeThumbnails();
	}

	itemsHeightCalc();
	revertModel();
	repaintItems();
}

void ListJobs::getUserJobsOrder()
{
	std::ostringstream str;
	str <<  "{\"get\":{\"type\":\"users\",\"ids\":[" << MonitorHost::getUid() << "],\"mode\":\"jobs_order\",\"binary\":true}}";
	Watch::sendMsg( af::jsonMsg( str));
}

void ListJobs::calcTotals()
{
	int numjobs = count();
	int percent = 0;
	int done = 0;
	int running = 0;
	int error = 0;
	int blocksrun = 0;

	if (numjobs == 0)
	{
		this->setWindowTitleWithPrefix("Jobs: (none)");
		return;
	}

	for (int i = 0; i < numjobs; i++)
	{
		ItemJob * itemjob = static_cast<ItemJob*>(m_model->item(i));
		if (itemjob->state & AFJOB::STATE_DONE_MASK)
		{
			done++;
		}
		else
		{
			for (int b = 0; b < itemjob->getBlocksNum(); b++)
			{
				percent += itemjob->getBlockPercent(b);
				blocksrun++;
			}
		}
		if (itemjob->state & AFJOB::STATE_RUNNING_MASK) running ++;
		if (itemjob->state & AFJOB::STATE_ERROR_MASK  ) error   ++;
	}

	if (blocksrun)
	{
		QString title = QString("Jobs:%1").arg(numjobs);
		if (error)
			title += QString(" ERROR:%1").arg(error);
		title += QString(" Run:%1 %2%").arg(running).arg(percent / blocksrun);
		if (done)
			title += QString(" Done:%1").arg(done);
		this->setWindowTitleWithPrefix(title);
	}
	else
		this->setWindowTitleWithPrefix(QString("Jobs: %1 Done").arg(numjobs));
}

void ListJobs::actMoveUp()     { moveJobs("move_jobs_up"    ); }
void ListJobs::actMoveDown()   { moveJobs("move_jobs_down"  ); }
void ListJobs::actMoveTop()    { moveJobs("move_jobs_top"   ); }
void ListJobs::actMoveBottom() { moveJobs("move_jobs_bottom"); }
void ListJobs::moveJobs( const std::string & i_operation)
{
	std::ostringstream str;
	std::vector<int> uids;
	uids.push_back( MonitorHost::getUid());
	af::jsonActionOperationStart( str, "users", i_operation, "", uids);
	Item::EType type = Item::TJob;
	std::vector<int> jids = getSelectedIds(type);
	str << ",\n\"jids\":[";
	for( int i = 0; i < jids.size(); i++)
	{
		if( i ) str << ',';
		str << jids[i];
	}
	str << "]";
	af::jsonActionOperationFinish( str);
	Watch::sendMsg( af::jsonMsg( str));
	displayInfo( afqt::stoq( i_operation));
}

void ListJobs::actStart()           { operation(Item::TAny, "start"            );}
void ListJobs::actStop()            { operation(Item::TJob, "stop"             );}
void ListJobs::actRestart()         { operation(Item::TJob, "restart"          );}
void ListJobs::actRestartErrors()   { operation(Item::TJob, "restart_errors"   );}
void ListJobs::actRestartWarnings() { operation(Item::TJob, "restart_warnings" );}
void ListJobs::actRestartRunning()  { operation(Item::TJob, "restart_running"  );}
void ListJobs::actRestartSkipped()  { operation(Item::TJob, "restart_skipped"  );}
void ListJobs::actRestartDone()     { operation(Item::TJob, "restart_done"     );}
void ListJobs::actResetErrorHosts() { operation(Item::TJob, "reset_error_hosts");}
void ListJobs::actPause()           { operation(Item::TAny, "pause"            );}
void ListJobs::actRestartPause()    { operation(Item::TJob, "restart_pause"    );}
void ListJobs::actDelete()          { operation(Item::TAny, "delete"           );}
void ListJobs::actResetTryingNextTasks(){operation(Item::TJob, "reset_trying_next_tasks");}

void ListJobs::actDeleteDone()
{
	std::vector<int> ids;
	for( int i = 0; i < m_model->count(); i++)
	{
		ItemJob * job = (ItemJob*)(m_model->item(i));
		if( job->state & AFJOB::STATE_DONE_MASK )
			ids.push_back( job->getId());
	}

	if( ids.size() == 0 )
	{
		displayWarning("No done jobs found.");
		return;
	}

	if (QMessageBox::question(this, "Delete Done Jobs", QString("Delete %1 done jobs?").arg(ids.size()),
				QMessageBox::No | QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
	{
		return;
	}

	std::ostringstream str;
	af::jsonActionOperation( str,"jobs","delete","", ids);
	Watch::sendMsg( af::jsonMsg( str));

	displayInfo("Delete all done jobs.");
}

void ListJobs::actRequestLog() { getItemInfo(Item::TAny, "log"); }
void ListJobs::actRequestErrorHostsList() { getItemInfo(Item::TAny, "error_hosts"); }

void ListJobs::actSetHidden()   {setParameter(Item::TJob, "hidden", "true" );}
void ListJobs::actUnsetHidden() {setParameter(Item::TJob, "hidden", "false");}

void ListJobs::actPreviewApproval()   {setParameter(Item::TJob, "ppa", "true" );}
void ListJobs::actNoPreviewApproval() {setParameter(Item::TJob, "ppa", "false");}

void ListJobs::actSetUser()
{
	ItemJob* item = (ItemJob*)getCurrentItem();
	if( item == NULL ) return;
	QString current = item->username;

	bool ok;
	QString text = QInputDialog::getText(this, "Change Owner", "Enter New User Name", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	setParameterQStr(Item::TJob, "user_name", text);
}

void ListJobs::actChangeBranch()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	QString current = jobitem->branch;
	bool ok;
	QString branch = QInputDialog::getText(this, "Change Branch", "Branch", QLineEdit::Normal, current, &ok);
	if( !ok) return;
	setParameterQStr(Item::TJob, "branch", branch);
}

void ListJobs::actPostCommand()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	QString current = jobitem->cmd_post;

	bool ok;
	QString cmd = QInputDialog::getText(this, "Change Post Command", "Enter Command", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	setParameterQStrEscape(Item::TJob, "command_post", cmd);
}

void ListJobs::v_doubleClicked(Item * i_item)
{
	if (i_item == NULL)
		return;

	if (i_item->getType() == Item::TJob)
		Watch::watchJobTasksWindowAdd(i_item->getId(), i_item->getName());
}

void ListJobs::actListenJob()
{
	Item* item = getCurrentItem();
	if( item == NULL ) return;
	if( Watch::isConnected()) Watch::listenJob( item->getId(), item->getName());
}

void ListJobs::actBrowseFolder( QString i_folder)
{
	af::Service service( afqt::qtos(i_folder));
	i_folder = afqt::stoq( service.getWDir());

	Watch::browseFolder( i_folder);
}

void ListJobs::actOpenRULES()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL )
		return;

	QString path = jobitem->getRulesFolder();
	if( path.isEmpty())
		return;

	QString cmd = QString("rules -s \"") + path + "\"";
	Watch::startProcess( cmd);
}

void ListJobs::slot_CollapseJobs() {collapseJobs(true );}
void ListJobs::slot_ExpandJobs()   {collapseJobs(false);}
void ListJobs::collapseJobs(bool i_collapse)
{
	afqt::QEnvironment::clearCollapsedJobSerials();

	for( int i = 0; i < m_model->count(); i++)
	{
		Item * item = m_model->item(i);
		if (item->getType() != Item::TJob)
			continue;

		ItemJob * item_job = static_cast<ItemJob*>(item);
		item_job->setItemCollapsed(i_collapse);
		m_view->emitSizeHintChanged(m_model->index(i));
	}
}

void ListJobs::slot_BlockAction(int i_bnum, QString i_json)
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if (jobitem == NULL) return;

	std::ostringstream str;
	Item::EType type = Item::TJob;
	std::vector<int> ids(getSelectedIds(type));
	af::jsonActionStart(str, "jobs", "", ids);

	if (m_all_blocks_menu_shown)
		str << ",\n\"block_ids\":[-1]";
	else
		str << ",\n\"block_ids\":[" << i_bnum << "]";

	str << ",\n" << afqt::qtos(i_json);

	af::jsonActionFinish(str);

	Watch::sendMsg(af::jsonMsg(str));
}

bool ListJobs::v_filesReceived( const af::MCTaskUp & i_taskup )
{
	if(( i_taskup.getNumBlock() != -1 ) || ( i_taskup.getNumTask() != -1 ))
		return false; // This is for a task (not for an entire job)

	for( int i = 0; i < count(); i++)
	{
		ItemJob * itemjob = (ItemJob*)(m_model->item(i));
		if( itemjob->getId() == i_taskup.getNumJob() )
		{
			itemjob->v_filesReceived( i_taskup);
			
			return true;
		}
	}

	return false;
}

