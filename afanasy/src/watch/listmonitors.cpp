#include "listmonitors.h"

#include "../libafanasy/address.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/monitor.h"
#include "../libafanasy/monitorevents.h"

#include "buttonpanel.h"
#include "ctrlsortfilter.h"
#include "itemmonitor.h"
#include "modelnodes.h"
#include "watch.h"

#include <QInputDialog>
#include <QLayout>
#include <QMenu>
#include <QtCore/QEvent>
#include <QtGui/QContextMenuEvent>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

int ListMonitors::ms_SortType1 = CtrlSortFilter::TNAME;
int ListMonitors::ms_SortType2 = CtrlSortFilter::TTIMEACTIVITY;
bool ListMonitors::ms_SortAscending1 = true;
bool ListMonitors::ms_SortAscending2 = true;
int ListMonitors::ms_FilterType = CtrlSortFilter::TNAME;
bool ListMonitors::ms_FilterInclude = true;
bool ListMonitors::ms_FilterMatch = false;
bool ListMonitors::ms_FilterRegEx = false;
std::string ListMonitors::ms_FilterString = "";

ListMonitors::ListMonitors(QWidget *parent) : ListNodes(parent, "monitors")
{
	m_ctrl_sf = new CtrlSortFilter(this, &ms_SortType1, &ms_SortAscending1, &ms_SortType2, &ms_SortAscending2,
								   &ms_FilterType, &ms_FilterInclude, &ms_FilterMatch, &ms_FilterRegEx,
								   &ms_FilterString);
	m_ctrl_sf->addSortType(CtrlSortFilter::TNONE);
	m_ctrl_sf->addSortType(CtrlSortFilter::TNAME);
	m_ctrl_sf->addSortType(CtrlSortFilter::TTIMELAUNCHED);
	m_ctrl_sf->addSortType(CtrlSortFilter::TTIMEREGISTERED);
	m_ctrl_sf->addSortType(CtrlSortFilter::TTIMEACTIVITY);
	m_ctrl_sf->addSortType(CtrlSortFilter::TENGINE);
	m_ctrl_sf->addSortType(CtrlSortFilter::TADDRESS);
	m_ctrl_sf->addFilterType(CtrlSortFilter::TNONE);
	m_ctrl_sf->addFilterType(CtrlSortFilter::TNAME);
	m_ctrl_sf->addFilterType(CtrlSortFilter::TENGINE);
	m_ctrl_sf->addFilterType(CtrlSortFilter::TADDRESS);
	initSortFilterCtrl();

	// Add left panel buttons:
	ButtonPanel *bp;

	bp = addButtonPanel(Item::TMonitor, "LOG", "monitors_log", "Show monitor log.");
	connect(bp, SIGNAL(sigClicked()), this, SLOT(actRequestLog()));

	bp = addButtonPanel(Item::TMonitor, "EXIT", "monitors_exit", "Exit monitor.", "", true);
	connect(bp, SIGNAL(sigClicked()), this, SLOT(actExit()));

	this->setWindowTitleWithPrefix("Monitors");

	initListNodes();
}

ListMonitors::~ListMonitors() {}

void ListMonitors::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu menu(this);
	QAction *action;

	action = new QAction("Show Log", this);
	connect(action, SIGNAL(triggered()), this, SLOT(actRequestLog()));
	menu.addAction(action);

	menu.addSeparator();

	action = new QAction("Send Message", this);
	connect(action, SIGNAL(triggered()), this, SLOT(actSendMessage()));
	menu.addAction(action);

	menu.addSeparator();

	action = new QAction("Exit Monitor", this);
	connect(action, SIGNAL(triggered()), this, SLOT(actExit()));
	menu.addAction(action);

	menu.exec(event->globalPos());
}

bool ListMonitors::v_caseMessage(af::Msg *msg)
{
#ifdef AFOUTPUT
	msg->stdOut();
#endif
	switch (msg->type())
	{
		case af::Msg::TMonitorsList:
		{
			updateItems(msg, Item::TMonitor);
			subscribe();
			calcTitle();
			break;
		}
		default: return false;
	}
	return true;
}

bool ListMonitors::v_processEvents(const af::MonitorEvents &i_me)
{
	if (i_me.m_events[af::Monitor::EVT_monitors_del].size())
	{
		deleteItems(i_me.m_events[af::Monitor::EVT_monitors_del], Item::TMonitor);
		calcTitle();
		return true;
	}

	std::vector<int> ids;

	for (int i = 0; i < i_me.m_events[af::Monitor::EVT_monitors_change].size(); i++)
		af::addUniqueToVect(ids, i_me.m_events[af::Monitor::EVT_monitors_change][i]);

	for (int i = 0; i < i_me.m_events[af::Monitor::EVT_monitors_add].size(); i++)
		af::addUniqueToVect(ids, i_me.m_events[af::Monitor::EVT_monitors_add][i]);

	if (ids.size())
	{
		get(ids);
		return true;
	}

	return false;
}

ItemNode *ListMonitors::v_createNewItemNode(af::Node *i_afnode, Item::EType i_type, bool i_notify)
{
	return new ItemMonitor(this, (af::Monitor *)i_afnode, m_ctrl_sf);
}

void ListMonitors::calcTitle()
{
	int total = count();
	int super = 0;
	for (int i = 0; i < total; i++)
	{
		ItemMonitor *itemmonitor = static_cast<ItemMonitor *>(m_model->item(i));
		if (itemmonitor->isSuperUser())
			super++;
	}
	this->setWindowTitleWithPrefix(QString("Monitors: %1, Super users %2").arg(total).arg(super));
}

void ListMonitors::actSendMessage()
{
	ItemMonitor *item = (ItemMonitor *)getCurrentItem();
	if (item == NULL)
		return;

	bool ok;
	QString text = QInputDialog::getText(this, "Send Message", "Enter Text", QLineEdit::Normal, "", &ok);
	if (!ok)
		return;

	std::ostringstream str;
	Item::EType type = Item::TAny;
	std::vector<int> ids(getSelectedIds(type));
	af::jsonActionOperationStart(str, "monitors", "message", "", ids);
	str << ",\n\"text\":\"" << af::strEscape(afqt::qtos(text)) << "\"";
	af::jsonActionOperationFinish(str);

	Watch::sendMsg(af::jsonMsg(str));
}

void ListMonitors::actRequestLog() { getItemInfo(Item::TAny, "log"); }

void ListMonitors::actExit() { operation(Item::TMonitor, "exit"); }
