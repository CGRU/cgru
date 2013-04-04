#include "listtalks.h"

#include "../libafanasy/address.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mctalkdistmessage.h"

#include "itemtalk.h"
#include "ctrlsortfilter.h"
#include "watch.h"

#include <QtGui/QMenu>
#include <QtCore/QEvent>
#include <QtGui/QInputDialog>
#include <QtGui/QLayout>
#include <QtGui/QContextMenuEvent>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

int	  ListTalks::SortType		 = CtrlSortFilter::TNAME;
bool	 ListTalks::SortAscending  = true;
int	  ListTalks::FilterType	  = CtrlSortFilter::TNAME;
bool	 ListTalks::FilterInclude  = true;
bool	 ListTalks::FilterMatch	 = false;
QString ListTalks::FilterString	= "";

ListTalks::ListTalks( QWidget* parent):
	ListNodes(  parent, "talks", af::Msg::TTalksListRequest)
{
	ctrl = new CtrlSortFilter( this, &SortType, &SortAscending, &FilterType, &FilterInclude, &FilterMatch, &FilterString);
	ctrl->addSortType(	CtrlSortFilter::TNONE);
	ctrl->addSortType(	CtrlSortFilter::TNAME);
	ctrl->addFilterType( CtrlSortFilter::TNONE);
	ctrl->addFilterType( CtrlSortFilter::TNAME);
	initSortFilterCtrl();

	m_eventsShowHide << af::Msg::TMonitorTalksAdd;
	m_eventsOnOff	 << af::Msg::TMonitorTalksDel;

	m_parentWindow->setWindowTitle("Talks");

	init();
}

ListTalks::~ListTalks()
{
}

void ListTalks::contextMenuEvent( QContextMenuEvent *event)
{
	QMenu menu(this);
	QAction *action;

	action = new QAction( "Send Message", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actSendMessage() ));
	menu.addAction( action);

	action = new QAction( "Exit Talk", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( actExit() ));
	menu.addAction( action);

	menu.exec( event->globalPos());
}

bool ListTalks::caseMessage( af::Msg * msg)
{
AFINFO("void ListTalks::caseMessage( Msg msg)\n");
#ifdef AFOUTPUT
	msg->stdOut();
#endif
	switch( msg->type())
	{
	case af::Msg::TTalksList:
	{
		updateItems( msg);
		v_subscribe();
		m_parentWindow->setWindowTitle(QString("Talks:%1").arg( count()));
		break;
	}
	case af::Msg::TMonitorTalksDel:
	{
		af::MCGeneral ids( msg);
		deleteItems( ids);
		m_parentWindow->setWindowTitle(QString("Talks:%1").arg( count()));
		break;
	}
	case af::Msg::TMonitorTalksAdd:
	{
		af::MCGeneral ids( msg);
		deleteItems( ids);
		Watch::sendMsg( new af::Msg( af::Msg::TTalksListRequestIds, &ids, true));
		break;
	}
	default:
		return false;
	}
	return true;
}

ItemNode* ListTalks::createNewItem( af::Node *node)
{
	return new ItemTalk( (af::Talk*)node);
}

void ListTalks::actSendMessage()
{
	ItemTalk* item = (ItemTalk*)getCurrentItem();
	if( item == NULL ) return;

	bool ok;
	QString text = QInputDialog::getText(this, "Send Message", "Enter Text", QLineEdit::Normal, "", &ok);
	if( !ok) return;

	af::MCTalkdistmessage mcdmsg( af::Environment::getUserName(), text.toUtf8().data());

	QList<Item*> items( getSelectedItems());
	if( items.count() < 1) return;

	for( int i = 0; i < items.count(); i++) mcdmsg.addUser(((ItemTalk*)(items[i]))->getUserName().toUtf8().data());

	Watch::sendMsg( new af::Msg( af::Msg::TTalkDistributeData, &mcdmsg));
}

void ListTalks::actExit() { operation("exit"); }
