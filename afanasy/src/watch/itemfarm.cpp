#include "itemfarm.h"

#include <QtGui/QPainter>

#include "listrenders.h"
#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

ItemFarm::ItemFarm(af::Node * i_afnode, Item::EType i_type, const CtrlSortFilter * i_ctrl_sf):
	ItemNode(i_afnode, i_type, i_ctrl_sf),
	m_depth(0)
{
}

ItemFarm::~ItemFarm()
{
}

void ItemFarm::updateFarmValues(af::Farm * i_affarm)
{
	// Grab services:
	m_services.clear();
	for (const std::string & s : i_affarm->m_services)
		m_services.append(afqt::stoq(s));

	// Grab disabled services:
	m_services_disabled.clear();
	for (const std::string & s : i_affarm->m_services_disabled)
		m_services_disabled.append(afqt::stoq(s));
}

void ItemFarm::setDepth(int i_depth)
{
	m_depth = i_depth;
	m_margin_left = DepthOffset * m_depth;
}

void ItemFarm::drawServices(QPainter * i_painter, int i_x, int i_y, int i_w, int i_h) const
{
	int x = i_x;
	// Iterate services:
	for (int i = 0; i < m_services.size(); i++)
	{
		int w = 4;

		// Draw icon:
		const QPixmap * icon = Watch::getServiceIconSmall(m_services[i]);
		if (icon)
		{
			i_painter->drawPixmap(x+2, i_y+2, *icon);
			w += 16;
		}

		// Draw name:
		i_painter->setFont(afqt::QEnvironment::f_info);
		QPen pen(afqt::QEnvironment::qclr_black);
		i_painter->setPen(pen);
		QRect bound;
		i_painter->drawText(x+w, i_y, i_w, i_h, Qt::AlignLeft | Qt::AlignVCenter, m_services[i], &bound);
		w += bound.width() + 4;

		// Draw border:
		i_painter->setPen(afqt::QEnvironment::clr_outline.c);
		i_painter->setBrush(Qt::NoBrush);
		i_painter->drawRect(x, i_y, w, i_h);

		x += w + 8;
	}

	int w = i_w;
	// Iterate disabled services:
	for (int i = 0; i < m_services_disabled.size(); i++)
	{
		int x = 4;

		// Draw name:
		i_painter->setFont(afqt::QEnvironment::f_info);
		QPen pen(afqt::QEnvironment::qclr_black);
		i_painter->setPen(pen);
		QRect bound;
		i_painter->drawText(i_x, i_y, w-x, i_h, Qt::AlignRight | Qt::AlignVCenter, m_services_disabled[i], &bound);
		x += bound.width() + 4;

		// Draw line:
		i_painter->setPen(afqt::QEnvironment::clr_error.c);
		i_painter->drawLine(i_x+w-x, i_y, i_x+w, i_y+i_h);

		// Draw border:
		i_painter->setPen(afqt::QEnvironment::clr_outline.c);
		i_painter->setBrush(Qt::NoBrush);
		i_painter->drawRect(i_x+w-x, i_y, x, i_h);

		w -= x + 8;
	}
}

