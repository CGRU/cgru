#include "listjobs.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/address.h"

#include "itemjob.h"
#include "ctrljobs.h"
#include "ctrlsortfilter.h"
#include "modelnodes.h"
#include "viewitems.h"
#include "watch.h"

#include <QtCore/QDateTime>
#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtGui/QBoxLayout>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QInputDialog>
#include <QtGui/QMenu>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

int     ListJobs::SortType       = CtrlSortFilter::TNONE;
bool    ListJobs::SortAscending  = false;
int     ListJobs::FilterType     = CtrlSortFilter::TNAME;
bool    ListJobs::FilterInclude  = true;
bool    ListJobs::FilterMatch    = false;
QString ListJobs::FilterString   = "";

int     ListJobs::SortType_SU       = CtrlSortFilter::TTIMECREATION;
bool    ListJobs::SortAscending_SU  = false;
int     ListJobs::FilterType_SU     = CtrlSortFilter::TUSERNAME;
bool    ListJobs::FilterInclude_SU  = true;
bool    ListJobs::FilterMatch_SU    = false;
QString ListJobs::FilterString_SU = "";

ListJobs::ListJobs( QWidget* parent):
	ListNodes( parent, "jobs")
{
	m_eventsShowHide << af::Msg::TMonitorJobsAdd;
	m_eventsShowHide << af::Msg::TMonitorJobsChanged;
	m_eventsOnOff    << af::Msg::TMonitorJobsDel;

	if( af::Environment::VISOR())
	{
		Watch::setUid( 0);
		ctrl = new CtrlSortFilter( this, &SortType_SU, &SortAscending_SU, &FilterType_SU, &FilterInclude_SU, &FilterMatch_SU, &FilterString_SU);
	}
	else
	{
		Watch::setUid( Watch::getUid());
		ctrl = new CtrlSortFilter( this, &SortType, &SortAscending, &FilterType, &FilterInclude, &FilterMatch, &FilterString);
	}

	ctrl->addSortType(   CtrlSortFilter::TNONE);
	ctrl->addSortType(   CtrlSortFilter::TTIMECREATION);
	ctrl->addSortType(   CtrlSortFilter::TTIMERUN);
	ctrl->addSortType(   CtrlSortFilter::TTIMESTARTED);
	ctrl->addSortType(   CtrlSortFilter::TTIMEFINISHED);
	ctrl->addSortType(   CtrlSortFilter::TNUMRUNNINGTASKS);
	ctrl->addSortType(   CtrlSortFilter::TNAME);
	ctrl->addFilterType( CtrlSortFilter::TNONE);
	ctrl->addFilterType( CtrlSortFilter::TNAME);
	if( af::Environment::VISOR())
	{
		ctrl->addSortType(   CtrlSortFilter::TPRIORITY);
		ctrl->addSortType(   CtrlSortFilter::THOSTNAME);
		ctrl->addSortType(   CtrlSortFilter::TUSERNAME);
		ctrl->addFilterType( CtrlSortFilter::THOSTNAME);
		ctrl->addFilterType( CtrlSortFilter::TUSERNAME);
	}

	initSortFilterCtrl();

	CtrlJobs * control = new CtrlJobs( ctrl, this);
	control->setToolTip("\
Sort & Filter Jobs.\n\
Press RMB for Options.\
");
	ctrl->getLayout()->addWidget( control);

	init();

	QTimer * timer = new QTimer(this);
	timer->start( 1000 * af::Environment::getWatchRefreshInterval());
	connect( timer, SIGNAL( timeout()), this, SLOT( repaintItems()));

	m_parentWindow->setWindowTitle("Jobs:");
}

void ListJobs::v_shownFunc()
{
	if( Watch::isConnected() == false) return;

	if( af::Environment::VISOR())
		Watch::sendMsg( new af::Msg( af::Msg::TJobsListRequest, 0, true));
	else
	{
		if( Watch::getUid())
			Watch::sendMsg( new af::Msg( af::Msg::TJobsListRequestUserId, Watch::getUid(), true));
		else
			if( m_parentWindow != (QWidget*)Watch::getDialog()) close();
	}
}

void ListJobs::v_connectionLost()
{
	if( m_parentWindow != (QWidget*)Watch::getDialog()) m_parentWindow->close();
}

void ListJobs::contextMenuEvent( QContextMenuEvent *event)
{
	QMenu menu(this);
	QMenu * submenu;
	QAction *action;

	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	int selectedItemsCount = getSelectedItemsCount();

	action = new QAction( "Show Log", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actRequestLog() ));
	menu.addAction( action);

	action = new QAction( "Show Error Hosts", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actRequestErrorHostsList() ));
	menu.addAction( action);

	action = new QAction( "Reset Error Hosts", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actResetErrorHosts() ));
	menu.addAction( action);

	action = new QAction( "Restart Errors", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actRestartErrors() ));
	if( selectedItemsCount == 1) action->setEnabled( jobitem->state & AFJOB::STATE_ERROR_MASK);
	menu.addAction( action);

	action = new QAction( "Restart Running", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actRestartRunning() ));
	if( selectedItemsCount == 1) action->setEnabled( jobitem->state & AFJOB::STATE_RUNNING_MASK);
	menu.addAction( action);

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

	action = new QAction( "Annotate", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actAnnotate() ));
	menu.addAction( action);

	submenu = new QMenu( "Set Parameter", this);

	action = new QAction( "Hidden", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actSetHidden() ));
	submenu->addAction( action);
	action = new QAction( "Not Hidden", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actUnsetHidden() ));
	submenu->addAction( action);
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

	action = new QAction( "Start", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actStart()   ));
	if( selectedItemsCount == 1) action->setEnabled( jobitem->state & AFJOB::STATE_OFFLINE_MASK);
	menu.addAction( action);

	action = new QAction( "Pause", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actPause()   ));
	if( selectedItemsCount == 1) action->setEnabled( false == (jobitem->state & AFJOB::STATE_OFFLINE_MASK));
	menu.addAction( action);

	action = new QAction( "Stop", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actStop()    ));
	if( selectedItemsCount == 1) action->setEnabled( jobitem->state & AFJOB::STATE_RUNNING_MASK);
	menu.addAction( action);

	action = new QAction( "Restart", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actRestart() ));
	if( selectedItemsCount == 1) action->setEnabled( jobitem->time_started != 0);
	menu.addAction( action);

	action = new QAction( "Restart&&Pause", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actRestartPause() ));
	if( selectedItemsCount == 1) action->setEnabled( jobitem->time_started != 0);
	menu.addAction( action);

	menu.addSeparator();

	action = new QAction( "Listen", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actListenJob() ));
	menu.addAction( action);

	menu.addSeparator();

	// System job ID is 1, and can not be deleted
	if( jobitem->getId() != 1 )
	{
		action = new QAction( "Delete", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actDelete()  ));
		menu.addAction( action);
	}

	menu.exec(event->globalPos());
}

ListJobs::~ListJobs()
{
#ifdef AFOUTPUT
printf("ListJobs::~ListJobs:\n");
#endif
}

bool ListJobs::caseMessage( af::Msg * msg)
{
#ifdef AFOUTPUT
printf("ListJobs::caseMessage:\n"); msg->stdOut();
#endif
	switch( msg->type())
	{
	case af::Msg::TJobsList:
	{
		if( updateItems( msg) && (af::Environment::VISOR() == false))
		{
			Watch::sendMsg( new af::Msg( af::Msg::TUserJobsOrderRequestId, Watch::getUid(), true));
		}
		if( false == isSubscribed() )
		{
			if( af::Environment::VISOR() == false )
			{
				m_view->scrollToBottom();
			}
			v_subscribe();
		}
		break;
	}
	case af::Msg::TMonitorJobsDel:
	{
		af::MCGeneral ids( msg);
		deleteItems( ids);
		break;
	}
	case af::Msg::TMonitorJobsAdd:
	{
		af::MCGeneral ids( msg);
		deleteItems( ids);
		Watch::sendMsg( new af::Msg( af::Msg::TJobsListRequestIds, &ids, true));
		Watch::someJobAdded();
		break;
	}
	case af::Msg::TMonitorJobsChanged:
	{
		af::MCGeneral ids( msg);
		Watch::sendMsg( new af::Msg( af::Msg::TJobsListRequestIds, &ids, true));
		break;
	}
	case af::Msg::TUserJobsOrder:
	{
		af::MCGeneral ids( msg);
		if( ids.getId() != Watch::getUid()) return true;
		sortMatch( ids.getList());
		break;
	}
	default:
		return false;
	}

	calcTotals();

	return true;
}

ItemNode* ListJobs::createNewItem( af::Node *node)
{
	return new ItemJob( (af::Job*)node);
}

void ListJobs::resetSorting()
{
	if( af::Environment::VISOR() == false )
		Watch::sendMsg( new af::Msg( af::Msg::TUserJobsOrderRequestId, Watch::getUid(), true));
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
	uids.push_back( Watch::getUid());
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
void ListJobs::actRestartRunning()  { operation("restart_running"  );}
void ListJobs::actResetErrorHosts() { operation("reset_error_hosts");}
void ListJobs::actPause()           { operation("pause"            );}
void ListJobs::actRestartPause()    { operation("restart_pause"    );}
void ListJobs::actDelete()          { operation("delete"           );}

void ListJobs::actRequestLog()
{
	displayInfo( "Job log request.");
	Item* item = getCurrentItem();
	if( item == NULL ) return;
	af::Msg * msg = new af::Msg( af::Msg::TJobLogRequestId, item->getId(), true);
	Watch::sendMsg( msg);
}

void ListJobs::actRequestErrorHostsList()
{
	displayInfo( "Job void hosts request.");
	Item* jobitem = getCurrentItem();
	if( jobitem == NULL ) return;
	af::Msg * msg = new af::Msg( af::Msg::TJobErrorHostsRequestId, jobitem->getId(), true);
	Watch::sendMsg( msg);
}

void ListJobs::actSetHidden()   { setParameter("hidden", "true",  false); }
void ListJobs::actUnsetHidden() { setParameter("hidden", "false", false); }

void ListJobs::actAnnotate()
{
	ItemJob* item = (ItemJob*)getCurrentItem();
	if( item == NULL ) return;
	QString current = item->annotation;

	bool ok;
	QString text = QInputDialog::getText(this, "Annotate", "Enter Annotation", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	setParameter("annotation", afqt::qtos( text));
}

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

void ListJobs::actPriority()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	int current = jobitem->priority;

	int maximum = 250;
	bool ok;
	int priority = QInputDialog::getInteger(this, "Change Priority", "Enter New Priority", current, 0, maximum, 1, &ok);
	if( !ok) return;

	setParameter("priority", priority);
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
	int max = QInputDialog::getInteger(this, "Change Maximum Running Tasks", "Enter Number", current, -1, 999999, 1, &ok);
	if( !ok) return;

	setParameter("max_running_tasks", max);
}

void ListJobs::actMaxRunTasksPerHost()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	int current = jobitem->maxruntasksperhost;

	bool ok;
	int max = QInputDialog::getInteger(this, "Change Maximum Running Tasks Per Host", "Enter Number", current, -1, 999999, 1, &ok);
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

	QRegExp rx( mask, Qt::CaseInsensitive);
	if( rx.isValid() == false )
	{
		displayError( rx.errorString());
		return;
	}

	setParameter("hosts_mask", afqt::qtos( mask));
}

void ListJobs::actHostsMaskExclude()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	QString current = jobitem->hostsmask_exclude;

	bool ok;
	QString mask = QInputDialog::getText(this, "Change Exclude Mask", "Enter New Mask", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	QRegExp rx( mask, Qt::CaseInsensitive);
	if( rx.isValid() == false )
	{
		displayError( rx.errorString());
		return;
	}

	setParameter("hosts_mask_exclude", afqt::qtos( mask));
}

void ListJobs::actDependMask()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	QString current = jobitem->dependmask;

	bool ok;
	QString mask = QInputDialog::getText(this, "Change Depend Mask", "Enter New Mask", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	QRegExp rx( mask, Qt::CaseInsensitive);
	if( rx.isValid() == false )
	{
		displayError( rx.errorString());
		return;
	}

	setParameter("depend_mask", afqt::qtos( mask));
}

void ListJobs::actDependMaskGlobal()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	QString current = jobitem->dependmask_global;

	bool ok;
	QString mask = QInputDialog::getText(this, "Change Depend Mask", "Enter New Mask", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	QRegExp rx( mask, Qt::CaseInsensitive);
	if( rx.isValid() == false )
	{
		displayError( rx.errorString());
		return;
	}

	setParameter("depend_mask_global", afqt::qtos( mask));
}

void ListJobs::actNeedOS()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	QString current = jobitem->need_os;

	bool ok;
	QString mask = QInputDialog::getText(this, "Change OS Needed", "Enter New Mask", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	QRegExp rx( mask, Qt::CaseInsensitive);
	if( rx.isValid() == false )
	{
		displayError( rx.errorString());
		return;
	}

	setParameter("need_os", afqt::qtos( mask));
}

void ListJobs::actNeedProperties()
{
	ItemJob* jobitem = (ItemJob*)getCurrentItem();
	if( jobitem == NULL ) return;
	QString current = jobitem->need_properties;

	bool ok;
	QString mask = QInputDialog::getText(this, "Change Properties Needed", "Enter New Mask", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	QRegExp rx( mask, Qt::CaseInsensitive);
	if( rx.isValid() == false )
	{
		displayError( rx.errorString());
		return;
	}

	setParameter("need_properties", afqt::qtos( mask));
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
