#include "itempool.h"

#include "../libafanasy/environment.h"

#include "../libafqt/qenvironment.h"

#include "ctrlsortfilter.h"
#include "listrenders.h"
#include "watch.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

const int ItemPool::HeightPool = 32;
const int ItemPool::HeightAnnotation = 14;

ItemPool::ItemPool(ListRenders * i_list_renders, af::Pool * i_pool, const CtrlSortFilter * i_ctrl_sf):
	ItemFarm(i_list_renders, i_pool, TPool, i_ctrl_sf),
	m_root(false)
{
	updateValues(i_pool, 0);
}

ItemPool::~ItemPool()
{
}

void ItemPool::v_updateValues(af::Node * i_afnode, int i_msgType)
{
	af::Pool * pool = (af::Pool*)i_afnode;

	updateNodeValues(pool);

	updateFarmValues(pool);

	m_root = pool->isRoot();

	setParentPath(afqt::stoq(pool->getParentPath()));
	m_sort_force = m_name;

	if (false == m_root)
		m_params["pattern"] = afqt::stoq(pool->getPatternStr());

	m_params["idle_free_time"]     = pool->m_idle_free_time;
	m_params["busy_nimby_time"]    = pool->m_busy_nimby_time;
	m_params["idle_cpu"]           = pool->m_idle_cpu;
	m_params["busy_cpu"]           = pool->m_busy_cpu;
	m_params["idle_mem"]           = pool->m_idle_mem;
	m_params["busy_mem"]           = pool->m_busy_mem;
	m_params["idle_swp"]           = pool->m_idle_swp;
	m_params["busy_swp"]           = pool->m_busy_swp;
	m_params["idle_hddgb"]         = pool->m_idle_hddgb;
	m_params["busy_hddgb"]         = pool->m_busy_hddgb;
	m_params["idle_hddio"]         = pool->m_idle_hddio;
	m_params["busy_hddio"]         = pool->m_busy_hddio;
	m_params["idle_netmbs"]        = pool->m_idle_netmbs;
	m_params["busy_netmbs"]        = pool->m_busy_netmbs;
	m_params["idle_wolsleep_time"] = pool->m_idle_wolsleep_time;
/*
	if (running_tasks_num)
		setRunning();
	else
		setNotRunning();
*/
	strLeftTop = m_name;
	if (false == m_root)
		strLeftTop += QString(": %1").arg(afqt::stoq(pool->getPatternStr()));
	strLeftBottom.clear();
	strRightTop.clear();

	if (Watch::isPadawan())
	{
		if (pool->getPoolsTotal())
			strLeftBottom += QString(" Pools Total: %1").arg(pool->getPoolsTotal());
		if (pool->getRendersTotal())
			strLeftBottom += QString(" Renders Total: %1").arg(pool->getRendersTotal());

		if (pool->isNewRenderNimby())
			strRightTop += " NewRender:Nimby";
		if (pool->isNewRenderPaused())
			strRightTop += " NewRender:Paused";
		if (pool->getMaxTasksHost() >= 0)
			strRightTop += QString(" HostMaxTasks:%1").arg(pool->getMaxTasksHost());
		if (pool->getCapacityHost() >= 0)
			strRightTop += QString(" HostCapacity:%1").arg(pool->getCapacityHost());
		strRightTop += QString(" Priority:%1").arg(pool->getPriority());
	}
	else if(Watch::isJedi())
	{
		if (pool->getPoolsTotal())
			strLeftBottom += QString(" Pools: %1").arg(pool->getPoolsTotal());
		if (pool->getRendersTotal())
			strLeftBottom += QString(" Renders: %1").arg(pool->getRendersTotal());

		if (pool->isNewRenderNimby())
			strRightTop += " New:Nimby";
		if (pool->isNewRenderPaused())
			strRightTop += " New:Paused";
		if (pool->getMaxTasksHost() >= 0)
			strRightTop += QString(" HostMaxTasks:%1").arg(pool->getMaxTasksHost());
		if (pool->getCapacityHost() >= 0)
			strRightTop += QString(" HostCapacity:%1").arg(pool->getCapacityHost());
		strRightTop += QString(" Priority:%1").arg(pool->getPriority());
	}
	else
	{
		if (pool->getPoolsTotal())
			strLeftBottom += QString(" p:%1").arg(pool->getPoolsTotal());
		if (pool->getRendersTotal())
			strLeftBottom += QString(" r:%1").arg(pool->getRendersTotal());

		if (pool->isNewRenderNimby())
			strRightTop += " n:Nimby";
		if (pool->isNewRenderPaused())
			strRightTop += " n:Paused";
		if (pool->getMaxTasksHost() >= 0)
			strRightTop += QString(" ht:%1").arg(pool->getMaxTasksHost());
		if (pool->getCapacityHost() >= 0)
			strRightTop += QString(" hc:%1").arg(pool->getCapacityHost());
		strRightTop += QString(" p:%1").arg(pool->getPriority());
	}

	m_idle_wolsleep_time = pool->m_idle_wolsleep_time;
	m_idle_free_time     = pool->m_idle_free_time;
	m_busy_nimby_time    = pool->m_busy_nimby_time;
	m_idle_cpu           = pool->m_idle_cpu;
	m_busy_cpu           = pool->m_busy_cpu;
	m_idle_mem           = pool->m_idle_mem;
	m_busy_mem           = pool->m_busy_mem;
	m_idle_swp           = pool->m_idle_swp;
	m_busy_swp           = pool->m_busy_swp;
	m_idle_hddgb         = pool->m_idle_hddgb;
	m_busy_hddgb         = pool->m_busy_hddgb;
	m_idle_hddio         = pool->m_idle_hddio;
	m_busy_hddio         = pool->m_busy_hddio;
	m_idle_netmbs        = pool->m_idle_netmbs;
	m_busy_netmbs        = pool->m_busy_netmbs;

	if (isLocked())
		strLeftTop += " (LOCKED)";

	m_tooltip = pool->v_generateInfoString(true).c_str();

	updateInfo(pool);

	calcHeight();
}

void ItemPool::updateInfo(af::Pool * i_pool)
{
	m_info_text.clear();

	m_info_text += QString("Pools Total: <b>%1</b><br>").arg(i_pool->getPoolsTotal());
	m_info_text += QString("Renders Total: <b>%1</b><br>").arg(i_pool->getRendersTotal());

	m_info_text += "<br>";
	m_info_text += QString("Created: <b>%1</b>").arg(afqt::time2Qstr(i_pool->getTimeCreation()));
}

bool ItemPool::calcHeight()
{
	int old_height = m_height;

	m_height = HeightPool;

	m_height += calcHeightFarm();

	if (m_annotation.size())
		m_height += HeightAnnotation;

	return old_height == m_height;
}

void ItemPool::v_paint(QPainter * i_painter, const QRect & i_rect, const QStyleOptionViewItem & i_option) const
{
	drawBack(i_painter, i_rect, i_option);
	int x = i_rect.x() + 5;
	int y = i_rect.y() + 2;
	int w = i_rect.width() - 10;
	int h = i_rect.height() - 4;
	int height_pool = HeightPool-2;

	i_painter->setPen(clrTextMain(i_option));
	i_painter->setFont(afqt::QEnvironment::f_name);
	i_painter->drawText(x, y, w, h, Qt::AlignLeft | Qt::AlignTop, strLeftTop);

	i_painter->setPen(clrTextInfo(i_option));
	i_painter->setFont(afqt::QEnvironment::f_info);
	i_painter->drawText(x, y, w, height_pool, Qt::AlignLeft  | Qt::AlignBottom, strLeftBottom);
	i_painter->drawText(x, y, w, height_pool, Qt::AlignRight | Qt::AlignBottom, strRightBottom);
	i_painter->setPen(afqt::QEnvironment::qclr_black);
	i_painter->drawText(x, y, w, height_pool, Qt::AlignRight | Qt::AlignTop,    strRightTop);

	y += HeightPool;

	if (m_services.size() || m_services_disabled.size())
	{
		drawServices(i_painter, x+6, y, w-6, HeightServices-6);
		y += HeightServices;
	}

	if (m_tickets_pool.size() || m_tickets_host.size())
	{
		drawTickets(i_painter, x+6, y, w-6, HeightTickets-6);
		y += HeightTickets;
	}

	if (m_annotation.size())
	{
		i_painter->setFont(afqt::QEnvironment::f_info);
		QPen pen(afqt::QEnvironment::qclr_black);
		i_painter->setPen(pen);
		i_painter->drawText(x, y-3, w, HeightAnnotation, Qt::AlignHCenter | Qt::AlignVCenter, m_annotation);
	}
}

void ItemPool::v_setSortType( int i_type1, int i_type2 )
{
	resetSorting();
}

void ItemPool::v_setFilterType( int i_type )
{
	resetFiltering();
}
