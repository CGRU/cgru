#include "itempool.h"

#include "../libafanasy/environment.h"

#include "../libafqt/qenvironment.h"

#include "ctrlsortfilter.h"
#include "itemrender.h"
#include "listrenders.h"
#include "watch.h"

#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

const int ItemPool::HeightPool = 32;
const int ItemPool::HeightPool_Small = 18;
const int ItemPool::HeightAnnotation = 14;

ItemPool::ItemPool(ListRenders *i_list_renders, af::Pool *i_pool, const CtrlSortFilter *i_ctrl_sf)
	: QObject(i_list_renders), ItemFarm(i_list_renders, i_pool, TPool, i_ctrl_sf), m_root(false),

	  m_resources_update_sec(-1), m_resources_update_period(-1), m_heartbeat_sec(-1),

	  m_paused(false)
{
	m_resources_update_timer = new QTimer(this);
	connect(m_resources_update_timer, SIGNAL(timeout()), this, SLOT(slot_RequestResources()));

	// Add buttons:
	addChildsHideShowButton();
	m_buttons_width = 16;

	updateValues(i_pool, 0);
}

ItemPool::~ItemPool() {}

void ItemPool::v_updateValues(af::Node *i_afnode, int i_msgType)
{
	af::Pool *pool = (af::Pool *)i_afnode;

	setHideFlag_Pools(true);

	updateNodeValues(pool);

	updateFarmValues(pool);

	m_root = pool->isRoot();
	m_busy = pool->isBusy();
	m_paused = pool->isPaused();

	if (m_busy)
		setRunning();
	else
		setNotRunning();

	setParentPath(afqt::stoq(pool->getParentPath()));
	m_sort_force = m_name;

	if (false == m_root)
		m_params["pattern"] = afqt::stoq(pool->getPatternStr());

	m_params["sick_errors_count"] = pool->getSickErrorsCount();

	m_params["idle_free_time"] = pool->m_idle_free_time;
	m_params["busy_nimby_time"] = pool->m_busy_nimby_time;
	m_params["idle_cpu"] = pool->m_idle_cpu;
	m_params["busy_cpu"] = pool->m_busy_cpu;
	m_params["idle_mem"] = pool->m_idle_mem;
	m_params["busy_mem"] = pool->m_busy_mem;
	m_params["idle_swp"] = pool->m_idle_swp;
	m_params["busy_swp"] = pool->m_busy_swp;
	m_params["idle_hddgb"] = pool->m_idle_hddgb;
	m_params["busy_hddgb"] = pool->m_busy_hddgb;
	m_params["idle_hddio"] = pool->m_idle_hddio;
	m_params["busy_hddio"] = pool->m_busy_hddio;
	m_params["idle_netmbs"] = pool->m_idle_netmbs;
	m_params["busy_netmbs"] = pool->m_busy_netmbs;
	m_params["idle_wolsleep_time"] = pool->m_idle_wolsleep_time;

	m_params["heartbeat_sec"] = pool->m_heartbeat_sec;
	m_params["resources_update_period"] = pool->m_resources_update_period;
	m_params["zombie_time"] = pool->m_zombie_time;
	m_params["exit_no_task_time"] = pool->m_exit_no_task_time;
	m_params["no_task_event_time"] = pool->m_no_task_event_time;
	m_params["overload_event_time"] = pool->m_overload_event_time;

	strLeftTop = m_name;
	if (false == m_root)
		strLeftTop += QString(": %1").arg(afqt::stoq(pool->getPatternStr()));
	strLeftBottom.clear();
	strRightTop.clear();
	strRightBottom.clear();

	if (pool->getPowerHost() != -1)
		strRightBottom += QString(" %1").arg(pool->getPowerHost());
	if (pool->getPropertiesHost().size())
		strRightBottom += QString(" %1").arg(afqt::stoq(pool->getPropertiesHost()));
	if (pool->getTaskStartFinishTime())
		strRightBottom += QString(" %1 %2")
							  .arg(pool->isBusy() ? "Busy" : "Free")
							  .arg(afqt::stoq(af::time2strHMS(time(NULL) - pool->getTaskStartFinishTime())));
	else
		strRightBottom += QString(" NEW");

	if (Watch::isPadawan())
	{
		if (pool->getPoolsTotal())
			strLeftBottom += QString(" Pools:%1").arg(pool->getPoolsTotal());

		if (pool->getRendersTotal())
			strLeftBottom += QString(" Renders:%1").arg(pool->getRendersTotal());
		if (pool->getRendersBusy())
			strLeftBottom += QString(" Busy:%1").arg(pool->getRendersBusy());
		if (pool->getRendersReady())
			strLeftBottom += QString(" Ready:%1").arg(pool->getRendersReady());
		if (pool->getRendersOnline())
			strLeftBottom += QString(" Online:%1").arg(pool->getRendersOnline());
		if (pool->getRendersOffline())
			strLeftBottom += QString(" Offline:%1").arg(pool->getRendersOffline());
		if (pool->getRendersNimby())
			strLeftBottom += QString(" Nimby:%1").arg(pool->getRendersNimby());
		if (pool->getRendersPaused())
			strLeftBottom += QString(" Paused:%1").arg(pool->getRendersPaused());
		if (pool->getRendersSick())
			strLeftBottom += QString(" Sick:%1").arg(pool->getRendersSick());

		if (pool->getRunTasks())
			strLeftBottom += QString(" RunningTasks:%1").arg(pool->getRunTasks());
		if (pool->getRunCapacity())
			strLeftBottom += QString(" RunningCapacity:%1").arg(pool->getRunCapacity());

		if (pool->isNewRenderNimby())
			strRightTop += " NewRender:Nimby";
		if (pool->isNewRenderPaused())
			strRightTop += " NewRender:Paused";
		if (pool->getMaxTasksHost() >= 0)
			strRightTop += QString(" HostMaxTasks:%1").arg(pool->getMaxTasksHost());
		if (pool->getCapacityHost() >= 0)
			strRightTop += QString(" HostCapacity:%1").arg(pool->getCapacityHost());
		if (pool->getSickErrorsCount() >= 0)
			strRightTop += QString(" SickErrors:%1").arg(pool->getSickErrorsCount());
	}
	else if (Watch::isJedi())
	{
		if (pool->getPoolsTotal())
			strLeftBottom += QString(" Pools:%1").arg(pool->getPoolsTotal());

		if (pool->getRendersTotal())
			strLeftBottom += QString(" Renders:%1").arg(pool->getRendersTotal());
		if (pool->getRendersBusy())
			strLeftBottom += QString(" Busy:%1").arg(pool->getRendersBusy());
		if (pool->getRendersReady())
			strLeftBottom += QString(" RDY:%1").arg(pool->getRendersReady());
		if (pool->getRendersOnline())
			strLeftBottom += QString(" ON:%1").arg(pool->getRendersOnline());
		if (pool->getRendersOffline())
			strLeftBottom += QString(" OFF:%1").arg(pool->getRendersOffline());
		if (pool->getRendersNimby())
			strLeftBottom += QString(" Nby:%1").arg(pool->getRendersNimby());
		if (pool->getRendersPaused())
			strLeftBottom += QString(" Pau:%1").arg(pool->getRendersPaused());
		if (pool->getRendersSick())
			strLeftBottom += QString(" Sick:%1").arg(pool->getRendersSick());

		if (pool->getRunTasks())
			strLeftBottom += QString(" Tasks:%1").arg(pool->getRunTasks());
		if (pool->getRunCapacity())
			strLeftBottom += QString(" Capacity:%1").arg(pool->getRunCapacity());

		if (pool->isNewRenderNimby())
			strRightTop += " New:Nimby";
		if (pool->isNewRenderPaused())
			strRightTop += " New:Paused";
		if (pool->getMaxTasksHost() >= 0)
			strRightTop += QString(" HostMaxTasks:%1").arg(pool->getMaxTasksHost());
		if (pool->getCapacityHost() >= 0)
			strRightTop += QString(" HostCapacity:%1").arg(pool->getCapacityHost());
		if (pool->getSickErrorsCount() >= 0)
			strRightTop += QString(" SickErr:%1").arg(pool->getSickErrorsCount());
	}
	else
	{
		if (pool->getPoolsTotal())
			strLeftBottom += QString(" P:%1").arg(pool->getPoolsTotal());

		if (pool->getRendersTotal())
			strLeftBottom += QString(" R:%1").arg(pool->getRendersTotal());
		if (pool->getRendersBusy())
			strLeftBottom += QString(" b:%1").arg(pool->getRendersBusy());
		if (pool->getRendersReady())
			strLeftBottom += QString(" r:%1").arg(pool->getRendersReady());
		if (pool->getRendersOnline())
			strLeftBottom += QString(" on:%1").arg(pool->getRendersOnline());
		if (pool->getRendersOffline())
			strLeftBottom += QString(" off:%1").arg(pool->getRendersOffline());
		if (pool->getRendersNimby())
			strLeftBottom += QString(" n:%1").arg(pool->getRendersNimby());
		if (pool->getRendersPaused())
			strLeftBottom += QString(" p:%1").arg(pool->getRendersPaused());
		if (pool->getRendersSick())
			strLeftBottom += QString(" s:%1").arg(pool->getRendersSick());

		strLeftBottom += QString(" t:%1").arg(pool->getRunTasks());
		if (pool->getRunCapacity())
			strLeftBottom += QString(" c:%1").arg(pool->getRunCapacity());

		if (pool->isNewRenderNimby())
			strRightTop += " n:Nimby";
		if (pool->isNewRenderPaused())
			strRightTop += " n:Paused";
		if (pool->getMaxTasksHost() >= 0)
			strRightTop += QString(" ht:%1").arg(pool->getMaxTasksHost());
		if (pool->getCapacityHost() >= 0)
			strRightTop += QString(" hc:%1").arg(pool->getCapacityHost());
		if (pool->getSickErrorsCount() >= 0)
			strRightTop += QString(" se:%1").arg(pool->getSickErrorsCount());
	}

	if (m_paused)
		strRightTop += " PAUSED";

	ItemNode::updateStrParameters(strRightTop);

	m_resources_update_period = pool->m_resources_update_period;
	m_heartbeat_sec = pool->m_heartbeat_sec;
	processResources();

	m_idle_wolsleep_time = pool->m_idle_wolsleep_time;
	m_idle_free_time = pool->m_idle_free_time;
	m_busy_nimby_time = pool->m_busy_nimby_time;

	if (isLocked())
		strLeftTop += " (LOCKED)";

	m_tooltip = pool->v_generateInfoString(true).c_str();

	updateInfo(pool);

	calcHeight();
}

void ItemPool::updateInfo(af::Pool *i_pool)
{
	m_info_text.clear();

	m_info_text += QString("Pools Total: <b>%1</b><br>").arg(i_pool->getPoolsTotal());
	m_info_text += QString("Renders Total: <b>%1</b><br>").arg(i_pool->getRendersTotal());

	m_info_text += "<br>";
	m_info_text += QString("Created: <b>%1</b>").arg(afqt::time2Qstr(i_pool->getTimeCreation()));

	ItemNode::updateInfo();
}

void ItemPool::processResources()
{
	int resources_update_period = get_resources_update_period();
	int heartbeat_sec = get_heartbeat_sec();

	if (resources_update_period <= 0)
		resources_update_period = AFRENDER::RESOURCES_UPDATE_PERIOD;
	if (heartbeat_sec <= 0)
		heartbeat_sec = AFRENDER::HEARTBEAT_SEC;

	if (m_resources_update_sec == resources_update_period * heartbeat_sec)
		return;

	m_resources_update_sec = resources_update_period * heartbeat_sec;
	m_resources_update_timer->start(1000 * m_resources_update_sec);

	for (int i = 0; i < m_child_list.size(); i++)
		if (m_child_list[i]->getType() == Item::TPool)
			(static_cast<ItemPool *>(m_child_list[i]))->processResources();
}

void ItemPool::slot_RequestResources()
{
	af::MCGeneral renders_ids;
	for (int i = 0; i < m_child_list.size(); i++)
	{
		if (m_child_list[i]->getType() != Item::TRender)
			continue;

		ItemRender *render = static_cast<ItemRender *>(m_child_list[i]);

		if (render->isOnline())
			renders_ids.addId(render->getId());
	}

	if (renders_ids.getCount())
		Watch::sendMsg(new af::Msg(af::Msg::TRendersResourcesRequestIds, &renders_ids));
}

bool ItemPool::calcHeight()
{
	int old_height = m_height;

	if (afqt::QEnvironment::render_item_size.n == ListRenders::ESmallSize)
	{
		m_height = HeightPool_Small;
	}
	else
	{
		m_height = HeightPool;

		if (m_services.size() || m_services_disabled.size() || m_running_services.size())
			m_height += HeightServices;

		if (m_tickets_pool.size() || m_tickets_host.size())
			m_height += HeightTickets;
	}
	if (m_annotation.size())
		m_height += HeightAnnotation;

	return old_height == m_height;
}

bool ItemPool::v_isSelectable() const
{
	if (af::Environment::GOD())
		return true;

	return false;
}

void ItemPool::v_paint(QPainter *i_painter, const QRect &i_rect, const QStyleOptionViewItem &i_option) const
{
	QColor c("#737770");
	QColor cb("#838780");
	QColor cp("#555555");
	//	const QColor * itemColor = &(afqt::QEnvironment::clr_itemrender.c);
	//	if (m_running_services.size()) itemColor = &(afqt::QEnvironment::clr_itemrenderbusy.c);
	const QColor *itemColor = &c;
	if (m_paused)
		itemColor = &cp;
	else if (m_busy)
		itemColor = &cb;

	drawBack(i_painter, i_rect, i_option, itemColor);
	int x = i_rect.x() + 5;
	int y = i_rect.y() + 2;
	int w = i_rect.width() - 10;
	int h = i_rect.height() - 4;
	int height_pool = HeightPool - 2;
	if (afqt::QEnvironment::render_item_size.n == ListRenders::ESmallSize)
	{
		y = i_rect.y();
		h = i_rect.height();
		height_pool = HeightPool_Small;
	}

	QRect qr_LeftTop;
	i_painter->setPen(clrTextMain(i_option));
	i_painter->setFont(afqt::QEnvironment::f_name);
	i_painter->drawText(x + m_buttons_width, y, w - m_buttons_width, h, Qt::AlignLeft | Qt::AlignTop,
						strLeftTop, &qr_LeftTop);

	i_painter->setPen(clrTextInfo(i_option));
	i_painter->setFont(afqt::QEnvironment::f_info);
	if (afqt::QEnvironment::render_item_size.n != ListRenders::ESmallSize)
		i_painter->drawText(x, y, w, height_pool, Qt::AlignLeft | Qt::AlignBottom, strLeftBottom);
	i_painter->setPen(afqt::QEnvironment::qclr_black);
	if (afqt::QEnvironment::render_item_size.n != ListRenders::ESmallSize)
		i_painter->drawText(x, y, w, height_pool, Qt::AlignRight | Qt::AlignBottom, strRightBottom);
	i_painter->drawText(x + m_buttons_width, y, w - m_buttons_width, height_pool,
						Qt::AlignRight | Qt::AlignTop, strRightTop);

	if (afqt::QEnvironment::render_item_size.n != ListRenders::ESmallSize)
	{
		y += HeightPool;

		if (m_services.size() || m_services_disabled.size() || m_running_services.size())
		{
			drawServices(i_painter, i_option, x + 6, y, w - 6, HeightServices - 6);
			y += HeightServices;
		}

		if (m_tickets_pool.size() || m_tickets_host.size())
		{
			drawTickets(i_painter, i_option, x + 6, y, w - 6, HeightTickets - 6);
			y += HeightTickets;
		}
	}
	else
	{
		int srv_w = 0;

		if (m_services.size() || m_running_services.size())
			srv_w += drawServices(i_painter, i_option, x + qr_LeftTop.width() + 6, y + 1, w - 6,
								  HeightServices - 8, false);

		if (m_tickets_pool.size() || m_tickets_host.size())
			drawTickets(i_painter, i_option, x + qr_LeftTop.width() + 6 + srv_w + 6, y + 1, w - 6,
						HeightTickets - 8, NULL, false);

		y += HeightPool_Small;
	}

	if (m_annotation.size())
	{
		i_painter->setFont(afqt::QEnvironment::f_info);
		QPen pen(afqt::QEnvironment::qclr_black);
		i_painter->setPen(pen);
		i_painter->drawText(x, y - 3, w, HeightAnnotation, Qt::AlignHCenter | Qt::AlignVCenter, m_annotation);
	}
}

void ItemPool::v_setSortType(int i_type1, int i_type2) { resetSorting(); }

void ItemPool::v_setFilterType(int i_type) { resetFiltering(); }
