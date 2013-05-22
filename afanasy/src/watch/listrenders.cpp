#include "listrenders.h"

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/address.h"

#include "actionid.h"
#include "dialog.h"
#include "itemrender.h"
#include "ctrlrenders.h"
#include "ctrlsortfilter.h"
#include "modelnodes.h"
#include "viewitems.h"
#include "watch.h"

#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QInputDialog>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMenu>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ListRenders::EDisplaySize ListRenders::ms_displaysize = ListRenders::EVariableSize;

int     ListRenders::SortType       = CtrlSortFilter::TPRIORITY;
bool    ListRenders::SortAscending  = false;
int     ListRenders::FilterType     = CtrlSortFilter::TNAME;
bool    ListRenders::FilterInclude  = true;
bool    ListRenders::FilterMatch    = false;
QString ListRenders::FilterString   = "";

ListRenders::ListRenders( QWidget* parent):
	ListNodes( parent, "renders", af::Msg::TRendersListRequest)
{
	ctrl = new CtrlSortFilter( this, &SortType, &SortAscending, &FilterType, &FilterInclude, &FilterMatch, &FilterString);
	ctrl->addSortType(   CtrlSortFilter::TNONE);
	ctrl->addSortType(   CtrlSortFilter::TPRIORITY);
	ctrl->addSortType(   CtrlSortFilter::TCAPACITY);
	ctrl->addSortType(   CtrlSortFilter::TTIMELAUNCHED);
	ctrl->addSortType(   CtrlSortFilter::TTIMEREGISTERED);
	ctrl->addSortType(   CtrlSortFilter::TNAME);
	ctrl->addSortType(   CtrlSortFilter::TTASKUSER);
	ctrl->addSortType(   CtrlSortFilter::TUSERNAME);
	ctrl->addSortType(   CtrlSortFilter::TVERSION);
	ctrl->addSortType(   CtrlSortFilter::TADDRESS);
	ctrl->addFilterType( CtrlSortFilter::TNONE);
	ctrl->addFilterType( CtrlSortFilter::TNAME);
	ctrl->addFilterType( CtrlSortFilter::TUSERNAME);
	ctrl->addFilterType( CtrlSortFilter::TTASKUSER);
	ctrl->addFilterType( CtrlSortFilter::TVERSION);
	ctrl->addFilterType( CtrlSortFilter::TADDRESS);
	initSortFilterCtrl();

	CtrlRenders * control = new CtrlRenders( ctrl, this);
	control->setToolTip("\
Sort & Filter Renders.\n\
Press RMB for Options.\
");
	ctrl->getLayout()->addWidget( control);

	m_eventsShowHide << af::Msg::TMonitorRendersAdd;
	m_eventsShowHide << af::Msg::TMonitorRendersChanged;
	m_eventsOnOff    << af::Msg::TMonitorRendersDel;

	timer = new QTimer( this);
	connect(timer, SIGNAL(timeout()), this, SLOT( requestResources()));
	timer->start( 990 * af::Environment::getRenderUpdateSec());

	m_parentWindow->setWindowTitle("Renders");

	init();

	connect( (ModelNodes*)m_model, SIGNAL(   nodeAdded( ItemNode *, const QModelIndex &)),
	                         this,   SLOT( renderAdded( ItemNode *, const QModelIndex &)));

	if( false == af::Environment::VISOR())
		connect( m_view->selectionModel(), SIGNAL( selectionChanged( const QItemSelection &, const QItemSelection &)),
	                                 this,   SLOT( selectionChanged( const QItemSelection &, const QItemSelection &)));

	setSpacing();

	QTimer * timer = new QTimer(this);
	timer->start( 1900 * af::Environment::getWatchRefreshInterval());
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
	 itemsHeightCahnged();
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
		if( qVariantCanConvert<Item*>( indexes[i].data()))
		{
			ItemRender * render = (ItemRender*)qVariantValue<Item*>( indexes[i].data());
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
		Watch::sendMsg( new af::Msg( af::Msg::TRendersResourcesRequestIds, &ids, true));
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

	if( me || af::Environment::VISOR())
	{
		menu.addSeparator();

		action = new QAction( "Set Priority", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actPriority() ));
		menu.addAction( action);
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

		action = new QAction( "Annotate", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actAnnotate() ));
		menu.addAction( action);
		action = new QAction( "Change Capacity", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actCapacity() ));
		menu.addAction( action);
		action = new QAction( "Change Max Tasks", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actMaxTasks() ));
		menu.addAction( action);
		action = new QAction( "Enable Service", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actEnableService() ));
		menu.addAction( action);
		action = new QAction( "Disable Service", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actDisableService() ));
		menu.addAction( action);
		action = new QAction( "Restore Defaults", this);
		if( selectedItemsCount == 1) action->setEnabled(render->isDirty());
		connect( action, SIGNAL( triggered() ), this, SLOT( actRestoreDefaults() ));
		menu.addAction( action);

		menu.addSeparator();

		{
			QMenu * submenu = new QMenu( "Eject", this);

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
	}

	if( af::Environment::GOD())
	{
		menu.addSeparator();

		action = new QAction( "Set Hidden", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actSetHidden() ));
		menu.addAction( action);
		action = new QAction( "Unset Hidden", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actUnsetHidden() ));
		menu.addAction( action);

		{
			QMenu * submenu = new QMenu( "Exit", this);

			action = new QAction( "Render", this);
			if( selectedItemsCount == 1) action->setEnabled(render->isOnline());
			connect( action, SIGNAL( triggered() ), this, SLOT( actExit() ));
			submenu->addAction( action);

			menu.addMenu( submenu);
		}

		action = new QAction( "Delete Render", this);
		connect( action, SIGNAL( triggered() ), this, SLOT( actDelete() ));
		if( selectedItemsCount == 1) action->setEnabled(false == render->isOnline());
		menu.addAction( action);
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

	menu.addSeparator();

	if( af::Environment::GOD())
	{
		{
			QMenu * submenu = new QMenu( "Wake-On-Lan", this);

			action = new QAction( "Sleep", this);
			if( selectedItemsCount == 1) action->setEnabled( render->isOnline() && ( false == render->isBusy()) && ( false == render->isWOLFalling()));
			connect( action, SIGNAL( triggered() ), this, SLOT( actWOLSleep()));
			submenu->addAction( action);
			action = new QAction( "Wake", this);
			if( selectedItemsCount == 1) action->setEnabled( render->isOffline());
			connect( action, SIGNAL( triggered() ), this, SLOT( actWOLWake()));
			submenu->addAction( action);

			menu.addMenu( submenu);
		}

		{
			QMenu * submenu = new QMenu( "Reboot", this);

			action = new QAction( "Computer", this);
			if( selectedItemsCount == 1) action->setEnabled(render->isOnline());
			connect( action, SIGNAL( triggered() ), this, SLOT( actReboot() ));
			submenu->addAction( action);

			menu.addMenu( submenu);
		}

		menu.addSeparator();

		{
			QMenu * submenu = new QMenu( "Shutdown", this);

			action = new QAction( "Computer", this);
			if( selectedItemsCount == 1) action->setEnabled(render->isOnline());
			connect( action, SIGNAL( triggered() ), this, SLOT( actShutdown() ));
			submenu->addAction( action);

			menu.addMenu( submenu);
		}
	}

	menu.exec( event->globalPos());
}

void ListRenders::doubleClicked( Item * item)
{
	actRequestInfo();
}

bool ListRenders::caseMessage( af::Msg * msg)
{
#ifdef AFOUTPUT
	msg->stdOut();
#endif
	switch( msg->type())
	{
	case af::Msg::TRendersList:
		v_subscribe();
	case af::Msg::TRendersResources:
	{
		updateItems( msg);
		calcTitle();
		break;
	}
	case af::Msg::TMonitorRendersDel:
	{
		af::MCGeneral ids( msg);
		deleteItems( ids);
		calcTitle();
		break;
	}
	case af::Msg::TMonitorRendersAdd:
	{
		af::MCGeneral ids( msg);
		deleteItems( ids);
		Watch::sendMsg( new af::Msg( af::Msg::TRendersListRequestIds, &ids, true));
		break;
	}
	case af::Msg::TMonitorRendersChanged:
	{
		af::MCGeneral ids( msg);
		Watch::sendMsg( new af::Msg( af::Msg::TRendersListRequestIds, &ids, true));
		break;
	}
	default:
		return false;
	}
	return true;
}

ItemNode* ListRenders::createNewItem( af::Node *node)
{
	return new ItemRender( (af::Render*)node);
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

void ListRenders::actPriority()
{
	ItemRender* item = (ItemRender*)getCurrentItem();
	if( item == NULL ) return;
	int current = item->getPriority();

	bool ok;
	int priority = QInputDialog::getInteger(this, "Change Priority", "Enter New Priority", current, 0, 250, 1, &ok);
	if( !ok) return;
	setParameter("priority", priority);
}
void ListRenders::actCapacity()
{
	ItemRender* item = (ItemRender*)getCurrentItem();
	if( item == NULL ) return;
	int current = item->getCapacity();

	bool ok;
	int capacity = QInputDialog::getInteger(this, "Change Capacity", "Enter New Capacity", current, -1, 1000000, 1, &ok);
	if( !ok) return;
	setParameter("capacity", capacity);
}
void ListRenders::actMaxTasks()
{
	ItemRender* item = (ItemRender*)getCurrentItem();
	if( item == NULL ) return;
	int current = item->getMaxTasks();

	bool ok;
	int max_tasks = QInputDialog::getInteger(this, "Change Maximum Tasksy", "Enter New Limit", current, -1, 1000000, 1, &ok);
	if( !ok) return;
	setParameter("max_tasks", max_tasks);
}

void ListRenders::actNIMBY()       { setParameter("NIMBY",  "true",  false); }
void ListRenders::actNimby()       { setParameter("nimby",  "true",  false); }
void ListRenders::actFree()        { setParameter("nimby",  "false", false); }
void ListRenders::actSetHidden()   { setParameter("hidden", "true",  false); }
void ListRenders::actUnsetHidden() { setParameter("hidden", "false", false); }

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

void ListRenders::actRequestLog()
{
	Item* item = getCurrentItem();
	if( item == NULL ) return;
	displayInfo( "Render log request.");
	af::Msg * msg = new af::Msg( af::Msg::TRenderLogRequestId, item->getId(), true);
	Watch::sendMsg( msg);
}

void ListRenders::actRequestTasksLog()
{
	Item* item = getCurrentItem();
	if( item == NULL ) return;
	displayInfo( "Render tasks log request.");
	af::Msg * msg = new af::Msg( af::Msg::TRenderTasksLogRequestId, item->getId(), true);
	Watch::sendMsg( msg);
}

void ListRenders::actRequestInfo()
{
	Item* item = getCurrentItem();
	if( item == NULL ) return;
	displayInfo( "Render info request.");
	af::Msg * msg = new af::Msg( af::Msg::TRenderInfoRequestId, item->getId(), true);
	Watch::sendMsg( msg);
}

void ListRenders::actAnnotate()
{
	ItemRender* item = (ItemRender*)getCurrentItem();
	if( item == NULL ) return;
	QString current = item->getAnnotation();

	bool ok;
	QString text = QInputDialog::getText(this, "Annotate", "Enter Annotation", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	setParameter("annotation", afqt::qtos( text));
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
	QString service = QInputDialog::getText(this, caption, "Enter Service Name", QLineEdit::Normal, QString(), &ok);
	if( !ok) return;

	std::ostringstream str;
	af::jsonActionOperationStart( str, "renders", "service", "", getSelectedIds());
	str << ",\n\"name\":\"" << afqt::qtos( service) << "\"";
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
		if( false == qVariantCanConvert<Item*>( indexes[i].data())) continue;
		ItemRender * item = (ItemRender*)(qVariantValue<Item*>( indexes[i].data()));
		if( item == NULL ) continue;
		QString final_command(cmd);
		final_command.replace( AFWATCH::CMDS_ARGUMENT, item->getName());
		final_command.replace( AFWATCH::CMDS_IPADDRESS, item->getIPString());
		Watch::startProcess( final_command);
	}
}
