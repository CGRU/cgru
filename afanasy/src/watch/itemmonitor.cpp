#include "itemmonitor.h"

#include "../libafanasy/msg.h"

#include "../libafqt/qenvironment.h"

#include "ctrlsortfilter.h"
#include "watch.h"

#include <QtCore/QEvent>
#include <QtNetwork/QHostAddress>
#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"


ItemMonitor::ItemMonitor(ListNodes * i_list_nodes, af::Monitor * i_monitor, const CtrlSortFilter * i_ctrl_sf):
	ItemNode(i_list_nodes, i_monitor, TMonitor, i_ctrl_sf)
{
	time_launch   = i_monitor->getTimeLaunch();
	time_register = i_monitor->getTimeRegister();

	if( Watch::isPadawan())
	{
		time_launch_str = QString("Time Launched: %1").arg( afqt::time2Qstr( time_launch  ));
		time_register_str = QString("Time Registered: %1").arg( afqt::time2Qstr( time_register));
		address_str = QString("IP Address: %1").arg( i_monitor->getAddress().generateIPString( true).c_str());
	}
	else if( Watch::isJedi())
	{
		time_launch_str = QString("Lan: %1").arg( afqt::time2Qstr( time_launch  ));
		time_register_str = QString("Reg: %1").arg( afqt::time2Qstr( time_register));
		address_str = QString("IP: %1").arg( i_monitor->getAddress().generateIPString( true).c_str());
	}
	else
	{
		time_launch_str = QString("l: %1").arg( afqt::time2Qstr( time_launch  ));
		time_register_str = QString("r: %1").arg( afqt::time2Qstr( time_register));
		address_str = i_monitor->getAddress().generateIPString( true).c_str();
	}

	engine = i_monitor->getEngine().c_str();

	updateValues(i_monitor, 0);
}

ItemMonitor::~ItemMonitor()
{
}

void ItemMonitor::v_updateValues(af::Node * i_afnode, int i_msgType)
{
	af::Monitor *monitor = (af::Monitor*)i_afnode;

	m_user_id = monitor->getUid();
	time_activity = monitor->getTimeActivity();

	events.clear();
	eventscount = 0;
	for( int e = 0; e < af::Monitor::EVT_COUNT; e++)
	{
		if( monitor->hasEvent(e))
		{
			events << af::Monitor::EVT_NAMES[e];
			eventscount ++;
		}
	}

	m_height = 25 + 12*eventscount;
	if( m_height < 75) m_height = 75;

	jobsids.clear();
	const std::list<int32_t> * jlist = monitor->getJobsIds();
	int jobsidscount = int( jlist->size());
	for( std::list<int32_t>::const_iterator it = jlist->begin(); it != jlist->end(); it++)
		jobsids += QString(" %1").arg( *it);

	if( Watch::isPadawan())
	{
		eventstitle   = QString("Subscribed Events[%1]:").arg( eventscount);
		m_user_id_str = QString("User Id: %1").arg( m_user_id);
		jobsidstitle  = QString("Opened Jobs Ids[%1]:").arg( jobsidscount);
		if( time_activity )
			time_activity_str = QString("Lates Activity Time: %1").arg( afqt::time2Qstr( time_activity ));
		else
			time_activity_str = QString("Lates Activity Time: %1").arg("No Activity");
	}
	else if( Watch::isJedi())
	{
		eventstitle   = QString("Events[%1]:").arg( eventscount);
		m_user_id_str = QString("User Id: %1").arg( m_user_id);
		jobsidstitle  = QString("Jobs Ids[%1]:").arg( jobsidscount);
		if( time_activity )
			time_activity_str = QString("Act: %1").arg( afqt::time2Qstr( time_activity ));
		else
			time_activity_str = "No Activity";
	}
	else
	{
		eventstitle   = QString("evts[%1]:").arg( eventscount);
		m_user_id_str = QString("uid: %1").arg( m_user_id);
		jobsidstitle  = QString("jids[%1]:").arg( jobsidscount);
		if( time_activity )
			time_activity_str = QString("a: %1").arg( afqt::time2Qstr( time_activity ));
		else
			time_activity_str = "n/a";
	}

	m_tooltip = afqt::stoq( monitor->v_generateInfoString( true));
	updateInfo(monitor);
}

void ItemMonitor::updateInfo(af::Monitor * i_monitor)
{
	m_info_text.clear();

	m_info_text += QString("Engine: <b>%1</b>").arg(afqt::stoq(i_monitor->getEngine()));
	m_info_text += "<br>";
	m_info_text += QString("<br>Launch Time: <b>%1</b>").arg(afqt::stoq(af::time2str(i_monitor->getTimeLaunch())));
	m_info_text += QString("<br>Register Time: <b>%1</b>").arg(afqt::stoq(af::time2str(i_monitor->getTimeRegister())));
	if (i_monitor->getTimeActivity())
		m_info_text += QString("<br>Time Activity: <b>%1</b>").arg(afqt::stoq(af::time2str(i_monitor->getTimeActivity())));

	m_info_text += "<br>";
	const std::list<int32_t> * jlist = i_monitor->getJobsIds();
	m_info_text += QString("<br>Job Ids[%1]:").arg(jlist->size());
	for (std::list<int32_t>::const_iterator it = jlist->begin(); it != jlist->end(); it++)
		m_info_text += QString(" <b>%1</b>").arg(*it);

	m_info_text += "<br>";
	m_info_text += "<br>Monitor Events:";
	for (int e = 0; e < af::Monitor::EVT_COUNT; e++)
	{
		m_info_text += QString(" <br> ") + af::Monitor::EVT_NAMES[e] + " : ";
		if (i_monitor->hasEvent(e))
			m_info_text += " <b>SUBMITTED</b>";
		else
			m_info_text += " ---";
	}
}

void ItemMonitor::v_paint(QPainter * i_painter, const QRect & i_rect, const QStyleOptionViewItem & i_option) const
{
	drawBack(i_painter, i_rect, i_option, isSuperUser() ? &(afqt::QEnvironment::clr_LinkVisited.c) : NULL);

	int x = i_rect.x(); int y = i_rect.y(); int w = i_rect.width(); int h = i_rect.height();

	i_painter->setPen(clrTextMain(i_option));
	i_painter->setFont(afqt::QEnvironment::f_name);
	i_painter->drawText(i_rect, Qt::AlignTop | Qt::AlignHCenter, m_name);

	i_painter->setPen(clrTextInfo(i_option));
	i_painter->setFont(afqt::QEnvironment::f_info);
	i_painter->drawText(x+10, y+15, eventstitle);
	for (int e = 0; e < eventscount; e++)
		i_painter->drawText( x+5, y+30+12*e, events[e]);

	i_painter->setPen(clrTextInfo(i_option));
	i_painter->setFont(afqt::QEnvironment::f_info);
	int i = y+2; int dy = 15;
	i_painter->drawText(x, i+=dy, w-5, h, Qt::AlignTop | Qt::AlignRight, time_launch_str);
	i_painter->drawText(x, i+=dy, w-5, h, Qt::AlignTop | Qt::AlignRight, time_register_str);
	i_painter->drawText(x, i+=dy, w-5, h, Qt::AlignTop | Qt::AlignRight, time_activity_str);

	i_painter->drawText(x, y, w-5, h, Qt::AlignBottom | Qt::AlignRight, address_str);

	i = y+2;
	i_painter->drawText(x, i+=dy, w-5, h, Qt::AlignTop | Qt::AlignHCenter, m_user_id_str);
	i_painter->drawText(x, i+=dy, w-5, h, Qt::AlignTop | Qt::AlignHCenter, jobsidstitle);
	i_painter->drawText(x, i+=dy, w-5, h, Qt::AlignTop | Qt::AlignHCenter, jobsids);

	i_painter->drawText(x, y+2, w-5, h, Qt::AlignTop | Qt::AlignRight, engine);
}

void ItemMonitor::v_setSortType( int i_type1, int i_type2 )
{
	resetSorting();

	switch( i_type1 )
	{
		case CtrlSortFilter::TNONE:
			break;
		case CtrlSortFilter::TNAME:
			m_sort_str1 = m_name;
			break;
		case CtrlSortFilter::TTIMELAUNCHED:
			m_sort_int1 = time_launch;
			break;
		case CtrlSortFilter::TTIMEREGISTERED:
			m_sort_int1 = time_register;
			break;
		case CtrlSortFilter::TTIMEACTIVITY:
			m_sort_int1 = time_activity;
			break;
		case CtrlSortFilter::TENGINE:
			m_sort_str1 = engine;
			break;
		case CtrlSortFilter::TADDRESS:
			m_sort_str1 = address_str;
			break;
		default:
			AF_ERR << "Invalid type1 number = " << i_type1;
	}

	switch( i_type2 )
	{
		case CtrlSortFilter::TNONE:
			break;
		case CtrlSortFilter::TNAME:
			m_sort_str2 = m_name;
			break;
		case CtrlSortFilter::TTIMELAUNCHED:
			m_sort_int2 = time_launch;
			break;
		case CtrlSortFilter::TTIMEREGISTERED:
			m_sort_int2 = time_register;
			break;
		case CtrlSortFilter::TTIMEACTIVITY:
			m_sort_int2 = time_activity;
			break;
		case CtrlSortFilter::TENGINE:
			m_sort_str2 = engine;
			break;
		case CtrlSortFilter::TADDRESS:
			m_sort_str2 = address_str;
			break;
		default:
			AF_ERR << "Invalid type2 number = " << i_type2;
	}
}

void ItemMonitor::v_setFilterType( int i_type )
{
	resetFiltering();

	switch( i_type )
	{
		case CtrlSortFilter::TNONE:
			break;
		case CtrlSortFilter::TNAME:
			m_filter_str = afqt::qtos( m_name);
			break;
		case CtrlSortFilter::TENGINE:
			m_filter_str = afqt::qtos( engine);
			break;
		case CtrlSortFilter::TADDRESS:
			m_filter_str = afqt::qtos( address_str);
			break;
		default:
			AF_ERR << "Invalid type number = " << i_type;
	}
}
