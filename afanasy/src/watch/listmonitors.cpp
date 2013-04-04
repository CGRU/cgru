#include "listmonitors.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/address.h"

#include "itemmonitor.h"
#include "ctrlsortfilter.h"
#include "modelitems.h"
#include "watch.h"

#include <QtGui/QMenu>
#include <QtCore/QEvent>
#include <QtGui/QInputDialog>
#include <QtGui/QLayout>
#include <QtGui/QContextMenuEvent>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

int     ListMonitors::SortType       = CtrlSortFilter::TNAME;
bool    ListMonitors::SortAscending  = true;
int     ListMonitors::FilterType     = CtrlSortFilter::TNAME;
bool    ListMonitors::FilterInclude  = true;
bool    ListMonitors::FilterMatch    = false;
QString ListMonitors::FilterString   = "";

ListMonitors::ListMonitors( QWidget* parent):
	ListNodes(  parent, "monitors", af::Msg::TMonitorsListRequest)
{
	ctrl = new CtrlSortFilter( this, &SortType, &SortAscending, &FilterType, &FilterInclude, &FilterMatch, &FilterString);
	ctrl->addSortType(   CtrlSortFilter::TNONE);
	ctrl->addSortType(   CtrlSortFilter::TNAME);
	ctrl->addSortType(   CtrlSortFilter::TTIMELAUNCHED);
	ctrl->addSortType(   CtrlSortFilter::TTIMEREGISTERED);
	ctrl->addSortType(   CtrlSortFilter::TTIMEACTIVITY);
	ctrl->addSortType(   CtrlSortFilter::TVERSION);
	ctrl->addSortType(   CtrlSortFilter::TADDRESS);
	ctrl->addFilterType( CtrlSortFilter::TNONE);
	ctrl->addFilterType( CtrlSortFilter::TNAME);
	ctrl->addFilterType( CtrlSortFilter::TVERSION);
	ctrl->addFilterType( CtrlSortFilter::TADDRESS);
	initSortFilterCtrl();

	m_eventsShowHide << af::Msg::TMonitorMonitorsAdd;
	m_eventsShowHide << af::Msg::TMonitorMonitorsChanged;
	m_eventsOnOff    << af::Msg::TMonitorMonitorsDel;

	m_parentWindow->setWindowTitle("Monitors");

	init();
}

ListMonitors::~ListMonitors()
{
}

void ListMonitors::contextMenuEvent( QContextMenuEvent *event)
{
	QMenu menu(this);
	QAction *action;

	action = new QAction( "Send Message", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actSendMessage() ));
	menu.addAction( action);

	action = new QAction( "Exit Monitor", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actExit() ));
	menu.addAction( action);

	menu.exec( event->globalPos());
}

bool ListMonitors::caseMessage( af::Msg * msg)
{
AFINFO("ListMonitors::caseMessage( Msg msg)\n");
#ifdef AFOUTPUT
	msg->stdOut();
#endif
	switch( msg->type())
	{
	case af::Msg::TMonitorsList:
	{
		updateItems( msg);
		v_subscribe();
		calcTitle();
		break;
	}
	case af::Msg::TMonitorMonitorsDel:
	{
		af::MCGeneral ids( msg);
//printf("case af::Msg::TMonitorMonitorsDel: "); ids.stdOut( true);
		deleteItems( ids);
		calcTitle();
		break;
	}
	case af::Msg::TMonitorMonitorsAdd:
	{
		af::MCGeneral ids( msg);
//printf("case af::Msg::TMonitorMonitorsAdd: "); ids.stdOut( true);
		deleteItems( ids);
		Watch::sendMsg( new af::Msg( af::Msg::TMonitorsListRequestIds, &ids, true));
		break;
	}
	case af::Msg::TMonitorMonitorsChanged:
	{
		af::MCGeneral ids( msg);
//printf("case af::Msg::TMonitorMonitorsChanged: "); ids.stdOut( true);
		Watch::sendMsg( new af::Msg( af::Msg::TMonitorsListRequestIds, &ids, true));
		break;
	}
	default:
		return false;
	}
	return true;
}

ItemNode* ListMonitors::createNewItem( af::Node *node)
{
	return new ItemMonitor( (af::Monitor*)node);
}

void ListMonitors::calcTitle()
{
	int total = count();
	int super = 0;
	for( int i = 0; i < total; i++)
	{
		ItemMonitor * itemmonitor = (ItemMonitor*)(m_model->item(i));
		if( itemmonitor->superuser ) super++;
	}
	m_parentWindow->setWindowTitle(QString("M[%1]: %2S").arg( total).arg( super));
}

void ListMonitors::actSendMessage()
{
	ItemMonitor* item = (ItemMonitor*)getCurrentItem();
	if( item == NULL ) return;

	bool ok;
	QString text = QInputDialog::getText(this, "Send Message", "Enter Text", QLineEdit::Normal, "", &ok);
	if( !ok) return;

	af::MCGeneral mcgeneral( text.toUtf8().data());
	action( mcgeneral, af::Msg::TMonitorMessage);
}

void ListMonitors::actExit() { operation("exit"); }

