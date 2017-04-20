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
#include "viewitems.h"
#include "watch.h"

#include <QtCore/QDateTime>
#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtGui/QContextMenuEvent>
#include <QBoxLayout>
#include <QInputDialog>
#include <QMenu>

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

ListJobs::ListJobs( QWidget* parent):
	ListNodes( parent, "jobs")
{
	if( af::Environment::VISOR())
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

	initSortFilterCtrl();

	CtrlJobs * control = new CtrlJobs( m_ctrl_sf, this);
	m_ctrl_sf->getLayout()->addWidget( control);


	// Add left panel buttons:
	ButtonPanel * bp;

	bp = addButtonPanel("LOG","jobs_log","Show job log.");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actRequestLog()));

	bp = addButtonPanel("EHO","jobs_show_err_hosts","Show error hosts.");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actRequestErrorHostsList()));

	bp = addButtonPanel("PAU","jobs_pause","Pause selected jobs.","P");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actPause()));

	bp = addButtonPanel("STA","jobs_start","Start selected jobs.","S");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actStart()));

	bp = addButtonPanel("REH","jobs_reset_avoid_hosts","Reset error hosts.","E");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actResetErrorHosts()));

	bp = addButtonPanel("RET","jobs_restart_error_tasks","Restart error tasks.","R");
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actRestartErrors()));

	bp = addButtonPanel("DEL","jobs_delete","Delete selected jobs.","D", true);
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actDelete()));

	bp = addButtonPanel("DDJ","jobs_delete_done","Delete all done jobs.","", true);
	connect( bp, SIGNAL( sigClicked()), this, SLOT( actDeleteDone()));


	init();

	QTimer * timer = new QTimer(this);
	timer->start( 1000 * af::Environment::getWatchRefreshGuiSec());
	connect( timer, SIGNAL( timeout()), this, SLOT( repaintItems()));

	m_parentWindow->setWindowTitle("Jobs:");
}

void ListJobs::v_showFunc()
{
	if( Watch::isConnected() == false) return;

	if( af::Environment::VISOR())
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

	action = new QAction( "Max Running Tasks", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actMaxRunningTasks() ));
	submenu->addAction( action);
	action = new QAction( "Max Run Tasks Per Host", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actMaxRunTasksPerHost() ));
	submenu->addAction( action);
	action = new QAction( "Hosts Mask", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actHostsMask() ));
	submenu->addAction( action);
	action = new QAction( "Hosts Exclude Mask", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actHostsMaskExclude() ));
	submenu->addAction( action);

	if(( af::Environment::VISOR()) || ( af::Environment::getPermUserModJobPriority()))
	{
	   action = new QAction( "Priority", this);
	   connect( action, SIGNAL( triggered() ), this, SLOT( actPriority() ));
	   submenu->addAction( action);
	}

	action = new QAction( "Depend Mask", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actDependMask() ));
	submenu->addAction( action);
	action = new QAction( "Global Depend Mask", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actDependMaskGlobal() ));
	submenu->addAction( action);
	action = new QAction( "Wait Time", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actWaitTime() ));
	submenu->addAction( action);
	action = new QAction( "Preview Approval", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actPreviewApproval() ));
	submenu->addAction( action);
	action = new QAction( "No Preview Approval", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actNoPreviewApproval() ));
	submenu->addAction( action);
	action = new QAction( "OS Needed", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actNeedOS() ));
	submenu->addAction( action);
	action = new QAction( "Properties Needed", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actNeedProperties() ));
	submenu->addAction( action);
	action = new QAction( "Post Command", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actPostCommand() ));
	submenu->addAction( action);
	action = new QAction( "Life Time", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actLifeTime() ));
	submenu->addAction( action);

	submenu->addSeparator();

	action = new QAction( "Hidden", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actSetHidden() ));
	submenu->addAction( action);
	action = new QAction( "Not Hidden", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actUnsetHidden() ));
	submenu->addAction( action);

	submenu->addSeparator();

	action = new QAction( "Custom Data", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actCustomData() ));
	submenu->addAction( action);

	menu.addMenu( submenu);

	menu.addSeparator();

	if( selectedItemsCount == 1)
	{
		if( jobitem->getBlocksNum() > 1)
		{
			submenu = new QMenu( "Tasks Blocks", this);
			for( int b = -1; b < jobitem->getBlocksNum(); b++)
			{
				QMenu * subsubmenu = new QMenu( b == -1 ? QString("_to_all_") : jobitem->getBlockName(b), this);
				jobitem->generateMenu( b, subsubmenu , this);
				submenu->addMenu( subsubmenu);
			}
		}
		else
		{
			submenu = new QMenu( "Tasks Block", this);
			jobitem->generateMenu( 0, submenu, this);
		}
	}
	else
	{
		submenu = new QMenu( "All Selected Blocks", this);
		jobitem->generateMenu( -1, submenu, this);
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
		action = new QAction( "Delete All Done", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actDeleteDone()));
		menu.addAction( action);

		action = new QAction( "Delete", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actDelete()));
		menu.addAction( action);
	}

	menu.exec(event->globalPos());
}

ListJobs::~ListJobs()
{
}

bool ListJobs::v_caseMessage( af::Msg * msg)
{
	switch( msg->type())
	{
	case af::Msg::TJobsList:
	{
		if( updateItems( msg) && (af::Environment::VISOR() == false))
		{
			getUserJobsOrder();
		}

		if( false == isSubscribed() )
		{
			if( af::Environment::VISOR() == false )
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
		af::MCGeneral ids( msg);
		AF_DEBUG << "Jobs order received: " << ids.v_generateInfoString( true);
		if( ids.getId() == MonitorHost::getUid())
			sortMatch( ids.getList());
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

	if( i_me.m_events[af::Monitor::EVT_jobs_del].size())
	{
		deleteItems( i_me.m_events[af::Monitor::EVT_jobs_del]);
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
		sortMatch( i_me.m_jobs_order_ids);
		processed = true;
	}

	return processed;
}

ItemNode * ListJobs::v_createNewItem( af::Node *node, bool i_subscibed)
{
	return new ItemJob( this, (af::Job*)node, i_subscibed, m_ctrl_sf);
}

void ListJobs::v_resetSorting()
{
	if( af::Environment::VISOR() == false )
		getUserJobsOrder();
}

void ListJobs::getUserJobsOrder()
{
	std::ostringstream str;
	str <<  "{\"get\":{\"type\":\"users\",\"ids\":[" << MonitorHost::getUid() << "],\"mode\":\"jobs_order\",\"binary\":true}}";
	Watch::sendMsg( af::jsonMsg( str));
}

void ListJobs::calcTotals()
{
	int percent = 0;
	int blocksrun = 0;
	int blocksdone = 0;
	int done = 0;
	int running = 0;
	int error = 0;

	int numjobs = count();
	if( numjobs == 0)
	{
		m_parentWindow->setWindowTitle("Jobs: (none)");
		return;
	}
	else
	{
		for( int i = 0; i < numjobs; i++)
		{
			ItemJob * itemjob = (ItemJob*)(m_model->item(i));
			if( itemjob->state & AFJOB::STATE_DONE_MASK )
			{
				done++;
				for( int b = 0; b < itemjob->getBlocksNum(); b++)
				{
					blocksdone++;
				}
			}
			else
			{
				for( int b = 0; b < itemjob->getBlocksNum(); b++)
				{
					percent += itemjob->getBlockPercent( b);
					blocksrun++;
				}
			}
			if( itemjob->state & AFJOB::STATE_RUNNING_MASK) running ++;
			if( itemjob->state & AFJOB::STATE_ERROR_MASK  ) error   ++;
		}
	}

	if( af::Environment::VISOR())
	{
		m_parentWindow->setWindowTitle(QString("J[%1]: R%2/%3D/%4E")
			.arg( numjobs).arg( running).arg( done).arg( error));
	}
	else
	{
		if( blocksrun )
			m_parentWindow->setWindowTitle(QString("J[%1]: R%2/%3D/%4E B%5-%6%")
				.arg( numjobs).arg( running).arg( done).arg( error).arg( blocksrun).arg( percent / blocksrun));
		else
			m_parentWindow->setWindowTitle(QString("J[%1]: Done").arg( numjobs));
	}
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
	std::vector<int> jids = getSelectedIds();
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

void ListJobs::actStart()           { operation("start"            );}
void ListJobs::actStop()            { operation("stop"             );}
void ListJobs::actRestart()         { operation("restart"          );}
void ListJobs::actRestartErrors()   { operation("restart_errors"   );}
void ListJobs::actRestartWarnings() { operation("restart_warnings" );}
void ListJobs::actRestartRunning()  { operation("restart_running"  );}
void ListJobs::actRestartSkipped()  { operation("restart_skipped"  );}
void ListJobs::actRestartDone()     { operation("restart_done"     );}
void ListJobs::actResetErrorHosts() { operation("reset_error_hosts");}
void ListJobs::actPause()           { operation("pause"            );}
void ListJobs::actRestartPause()    { operation("restart_pause"    );}
void ListJobs::actDelete()          { operation("delete"           );}

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
		displayWarning("No done jobs founded.");
		return;
	}

	std::ostringstream str;
	af::jsonActionOperation( str,"jobs","delete","", ids);
	Watch::sendMsg( af::jsonMsg( str));

	displayInfo("Delete all done jobs.");
}

void ListJobs::actRequestLog() { getItemInfo("log"); }
void ListJobs::actRequestErrorHostsList() { getItemInfo("error_hosts"); }

void ListJobs::actSetHidden()   { setParameter("hidden", "true",  false); }
void ListJobs::actUnsetHidden() { setParameter("hidden", "false", false); }

void ListJobs::actPreviewApproval()   { setParameter("ppa", "true",  false); }
void ListJobs::actNoPreviewApproval() { setParameter("ppa", "false", false); }

void ListJobs::actSetUser()
{
	ItemJob* item = (ItemJob*)getCurrentItem();
	if( item == NULL ) return;
	QString current = item->username;

	bool ok;
	QString text = QInputDialog::getText(this, "Change Owner", "Enter New User Name", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	setParameter("user_name", afqt::qtos( text));
}

void ListJobs::actWaitTime()
{
	static const QString format("yyyy.MM.dd HH:mm:ss");

	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;

	uint32_t waittime = jobitem->time_wait;
	if( waittime < time( NULL)) waittime = time( NULL);
	QString current = QDateTime::fromTime_t( waittime).toString( format);
	bool ok;
	QString waittime_str = QInputDialog::getText(this, "Set Wait Time", format, QLineEdit::Normal, current, &ok);
	if( !ok) return;

	waittime = QDateTime::fromString( waittime_str, format).toTime_t();
//printf("waittime_str=\"%s\", waittime=%d\n", waittime_str.toUtf8().data(), waittime);
	if( waittime == unsigned(-1) )
	{
		displayError( "Time format : " + format);
		return;
	}

	setParameter("time_wait", waittime);
}

void ListJobs::actMaxRunningTasks()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	int current = jobitem->maxrunningtasks;

	bool ok;
	int max = QInputDialog::getInt(this, "Change Maximum Running Tasks", "Enter Number", current, -1, 999999, 1, &ok);
	if( !ok) return;

	setParameter("max_running_tasks", max);
}

void ListJobs::actMaxRunTasksPerHost()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	int current = jobitem->maxruntasksperhost;

	bool ok;
	int max = QInputDialog::getInt(this, "Change Maximum Running Tasks Per Host", "Enter Number", current, -1, 999999, 1, &ok);
	if( !ok) return;

	setParameter("max_running_tasks_per_host", max);
}

void ListJobs::actHostsMask()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	QString current = jobitem->hostsmask;

	bool ok;
	QString mask = QInputDialog::getText(this, "Change Hosts Mask", "Enter New Mask", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	setParameterRE("hosts_mask", afqt::qtos( mask));
}

void ListJobs::actHostsMaskExclude()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	QString current = jobitem->hostsmask_exclude;

	bool ok;
	QString mask = QInputDialog::getText(this, "Change Exclude Mask", "Enter New Mask", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	setParameterRE("hosts_mask_exclude", afqt::qtos( mask));
}

void ListJobs::actDependMask()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	QString current = jobitem->dependmask;

	bool ok;
	QString mask = QInputDialog::getText(this, "Change Depend Mask", "Enter New Mask", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	setParameterRE("depend_mask", afqt::qtos( mask));
}

void ListJobs::actDependMaskGlobal()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	QString current = jobitem->dependmask_global;

	bool ok;
	QString mask = QInputDialog::getText(this, "Change Depend Mask", "Enter New Mask", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	setParameterRE("depend_mask_global", afqt::qtos( mask));
}

void ListJobs::actNeedOS()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	QString current = jobitem->need_os;

	bool ok;
	QString mask = QInputDialog::getText(this, "Change OS Needed", "Enter New Mask", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	setParameterRE("need_os", afqt::qtos( mask));
}

void ListJobs::actNeedProperties()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	QString current = jobitem->need_properties;

	bool ok;
	QString mask = QInputDialog::getText(this, "Change Properties Needed", "Enter New Mask", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	setParameterRE("need_properties", afqt::qtos( mask));
}

void ListJobs::actPostCommand()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	QString current = jobitem->cmd_post;

	bool ok;
	QString cmd = QInputDialog::getText(this, "Change Post Command", "Enter Command", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	setParameter("command_post", afqt::qtos( cmd));
}

void ListJobs::actLifeTime()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	double cur = double( jobitem->lifetime ) / (60.0*60.0);

	bool ok;
	double hours = QInputDialog::getDouble( this, "Life Time", "Enter number of hours (0=infinite)", cur, -1, 365*24, 3, &ok);
	if( !ok) return;

	int seconds = hours * 60.0 * 60.0;
	if( seconds < -1 ) seconds = -1;
	setParameter("time_life", seconds);
}

void ListJobs::doubleClicked( Item * item)
{
	if( item ) Watch::watchJodTasksWindowAdd( item->getId(), item->getName());
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

void ListJobs::blockAction( int id_block, QString i_action)
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;

	std::ostringstream str;
	af::jsonActionStart( str, "jobs", "", getSelectedIds());
	str << ",\n\"block_ids\":[" << id_block << ']';

	if( jobitem->blockAction( str, id_block, i_action, this))
	{
		af::jsonActionFinish( str);
		Watch::sendMsg( af::jsonMsg( str));
	}
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

