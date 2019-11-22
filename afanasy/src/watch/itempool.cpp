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
const int ItemPool::HeightServices = 24;
const int ItemPool::HeightAnnotation = 14;

ItemPool::ItemPool(af::Pool * i_pool, ListRenders * i_list_renders, const CtrlSortFilter * i_ctrl_sf):
	ItemNode((af::Node*)i_pool, TPool, i_ctrl_sf),
	m_ListRenders(i_list_renders),
	m_root(false),
	m_depth(0)
{
	v_updateValues(i_pool, 0);
}

ItemPool::~ItemPool()
{
}

void ItemPool::v_updateValues(af::Node * i_afnode, int i_msgType)
{
	af::Pool * pool = (af::Pool*)i_afnode;

	updateNodeValues(i_afnode);

	m_sort_force = m_name;

	m_root = pool->isRoot();
	m_parent_path = afqt::stoq(pool->getParentPath());

	// Grab services:
	m_services.clear();
	for (const std::string & s : pool->m_services)
		m_services.append(afqt::stoq(s));
	// Grab disabled services:
	m_services_disabled.clear();
	for (const std::string & s : pool->m_services_disabled)
		m_services_disabled.append(afqt::stoq(s));

	// Get parameters:
	if (false == m_root)
		m_params["pattern"] = afqt::stoq(pool->getPatternStr());
	m_params["max_tasks_per_host"] = pool->getMaxTasksPerHost();
	m_params["max_capacity_per_host"] = pool->getMaxCapacityPerHost();
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
		if (pool->getMaxTasksPerHost() >= 0)
			strRightTop += QString(" MaxTasksPerHost:%1").arg(pool->getMaxTasksPerHost());
		if (pool->getMaxCapacityPerHost() >= 0)
			strRightTop += QString(" MaxCapacityPerHost:%1").arg(pool->getMaxCapacityPerHost());
		strRightTop += QString(" Priority:%1").arg(pool->getPriority());
	}
	else if(Watch::isJedi())
	{
	}
	else
	{
	}

	if (isLocked())
		strLeftTop += " (LOCKED)";

	m_tooltip = pool->v_generateInfoString(true).c_str();

	updateInfo(pool);

	calcHeight();

	m_ListRenders->offsetHierarchy(this);
}

void ItemPool::setDepth(int i_depth)
{
	m_depth = i_depth;
	m_margin_left = ListRenders::ms_DepthOffset * m_depth;
}

void ItemPool::updateInfo(af::Pool * i_pool)
{
	m_info_text.clear();

	m_info_text += QString("Pools Total: <b>%1</b><br>").arg(i_pool->getPoolsTotal());
	m_info_text += QString("Renders Total: <b>%1</b><br>").arg(i_pool->getRendersTotal());

	m_info_text += "<br>";
	m_info_text += QString("Created at: <b>%1</b>").arg(afqt::time2Qstr(i_pool->getTimeCreation()));
}

bool ItemPool::calcHeight()
{
	int old_height = m_height;

	m_height = HeightPool;
	if (m_services.size() || m_services_disabled.size())
		m_height += HeightServices;
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

	if (m_services.size() || m_services_disabled.size())
		drawServices(i_painter, m_services, m_services_disabled,
				x+6, y+HeightPool, w-6, HeightServices-6);

	if (m_annotation.size())
	{
		i_painter->setFont(afqt::QEnvironment::f_info);
		QPen pen(afqt::QEnvironment::qclr_black);
		i_painter->setPen(pen);
		i_painter->drawText( x, y, w, h, Qt::AlignBottom | Qt::AlignHCenter, m_annotation);
	}
}

void ItemPool::setSortType( int i_type1, int i_type2 )
{
	resetSorting();
}

void ItemPool::setFilterType( int i_type )
{
	resetFiltering();
}
