#include "itemfarm.h"

#include <QtGui/QPainter>

#include "itempool.h"
#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

const int ItemFarm::HeightServices = 24;

ItemFarm::ItemFarm(ListNodes * i_list_nodes, af::Node * i_afnode, Item::EType i_type, const CtrlSortFilter * i_ctrl_sf):
	ItemNode(i_list_nodes, i_afnode, i_type, i_ctrl_sf),
	m_parent_pool(NULL)
{
}

ItemFarm::~ItemFarm()
{
}

void ItemFarm::updateFarmValues(af::Farm * i_affarm)
{
	// Take common parameters:
	m_params["max_tasks_host"]  = i_affarm->getMaxTasksHost();
	m_params["capacity_host"]   = i_affarm->getCapacityHost();
	m_params["power_host"]      = i_affarm->getPowerHost();
	m_params["properties_host"] = afqt::stoq(i_affarm->getPropertiesHost());

	// Grab services:
	m_services.clear();
	for (const std::string & s : i_affarm->m_services)
		m_services.append(afqt::stoq(s));

	// Grab disabled services:
	m_services_disabled.clear();
	for (const std::string & s : i_affarm->m_services_disabled)
		m_services_disabled.append(afqt::stoq(s));

	// Grab pool tickets:
	m_tickets_pool.clear();
	for (auto const & i : i_affarm->m_tickets_pool)
		m_tickets_pool[afqt::stoq(i.first)] = i.second;

	// Grab host tickets:
	m_tickets_host.clear();
	for (auto const & i : i_affarm->m_tickets_host)
		m_tickets_host[afqt::stoq(i.first)] = i.second;
}

void ItemFarm::v_parentItemChanged()
{
	ItemNode * node = getParentItem();

	if (node->getType() != Item::TPool)
	{
		AF_ERR << "Farm item '" << afqt::qtos(getName()) << "' parent is not a pool '" << afqt::qtos(node->getName()) << "'.";
		return;
	}

	m_parent_pool = static_cast<ItemPool*>(node);
}

void ItemFarm::drawServices(QPainter * i_painter, const QStyleOptionViewItem & i_option, int i_x, int i_y, int i_w, int i_h) const
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

		// Check if it exists in running services:
		int running_count = 0;
		QMap<QString, int>::const_iterator it = m_running_services.find(m_services[i]);
		if (it != m_running_services.end())
			running_count = it.value();

		// Draw name:
		QString name = m_services[i];
		if (running_count)
			name += QString(": %1").arg(running_count);
		i_painter->setFont(afqt::QEnvironment::f_info);
		QPen pen(Item::clrTextInfo(running_count, i_option.state & QStyle::State_Selected));
		i_painter->setPen(pen);
		QRect bound;
		i_painter->drawText(x+w, i_y, i_w, i_h, Qt::AlignLeft | Qt::AlignVCenter, name, &bound);
		w += bound.width() + 4;

		// Draw border:
		if (running_count)
			i_painter->setPen(afqt::QEnvironment::clr_running.c);
		else
			i_painter->setPen(afqt::QEnvironment::clr_outline.c);
		i_painter->setBrush(Qt::NoBrush);
		i_painter->drawRect(x, i_y, w, i_h);

		x += w + 8;
	}

	// Iterate running services:
	QMapIterator<QString, int> it(m_running_services);
	while (it.hasNext())
	{
		it.next();

		// Skip drawing, if it was shown above
		if (m_services.contains(it.key()))
			continue;

		int w = 4;

		// Draw icon:
		const QPixmap * icon = Watch::getServiceIconSmall(it.key());
		if (icon)
		{
			i_painter->drawPixmap(x+2, i_y+2, *icon);
			w += 16;
		}

		// Draw name:
		QString name = QString("%1: %2").arg(it.key()).arg(it.value());
		i_painter->setFont(afqt::QEnvironment::f_muted);
		QPen pen(Item::clrTextInfo(it.value(), i_option.state & QStyle::State_Selected));
		i_painter->setPen(pen);
		QRect bound;
		i_painter->drawText(x+w, i_y, i_w, i_h, Qt::AlignLeft | Qt::AlignVCenter, name, &bound);
		w += bound.width() + 4;

		// Draw border:
		QPen borderPen(Qt::DashDotDotLine);
		borderPen.setColor(afqt::QEnvironment::clr_running.c);
		i_painter->setPen(borderPen);
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

void ItemFarm::drawTickets(QPainter * i_painter, const QStyleOptionViewItem & i_option,
		int i_x, int i_y, int i_w, int i_h, int * o_tkhost_width) const
{
	int tkh_w = 0;
	int tkp_w = 0;

	if (getType() == Item::TPool)
	{
		QMapIterator<QString, af::Farm::Tiks> tkp_it(m_tickets_pool);
		while (tkp_it.hasNext())
		{
			tkp_it.next();

			int draw_flags = Item::TKD_LEFT | Item::TKD_BORDER;
			if (tkp_it.value().count == -1)
				draw_flags |= Item::TKD_DUMMY;

			QPen pen(Item::clrTextInfo(tkp_it.value().usage, i_option.state & QStyle::State_Selected));
			tkp_w += drawTicket(i_painter, pen, i_x + 5 + tkp_w, i_y, i_w - 10,
					draw_flags, tkp_it.key(),
					tkp_it.value().count, tkp_it.value().usage, tkp_it.value().hosts, tkp_it.value().max_hosts);

			tkp_w += 8;
		}

		QPen pen(afqt::QEnvironment::qclr_black);
		QMapIterator<QString, af::Farm::Tiks> tkh_it(m_tickets_host);
		while (tkh_it.hasNext())
		{
			tkh_it.next();

			tkh_w += drawTicket(i_painter, pen, i_x + 5, i_y + 3, i_w - 10 - tkh_w,
					Item::TKD_RIGHT, tkh_it.key(), tkh_it.value().count);

			tkh_w += 8;
		}
	}
	else
	{
		QMapIterator<QString, af::Farm::Tiks> tkh_it(m_tickets_host);
		while (tkh_it.hasNext())
		{
			tkh_it.next();

			int draw_flags = Item::TKD_RIGHT | Item::TKD_BORDER;

			int count = tkh_it.value().count;
			if (count == -1)
			{
				if (NULL == m_parent_pool)
					continue;

				count = m_parent_pool->getTicketHostCount(tkh_it.key());

				// Skip drawing host ticket, if farm has no limit (no ticket)
				if (count == -1)
					continue;

				draw_flags |= Item::TKD_DUMMY;
			}

			QPen pen(Item::clrTextInfo(tkh_it.value().usage, i_option.state & QStyle::State_Selected));

			tkh_w += drawTicket(i_painter, pen, i_x + 5, i_y, i_w - 10 - tkh_w,
					draw_flags, tkh_it.key(), count, tkh_it.value().usage);

			tkh_w += 8;
		}
	}

	if (o_tkhost_width)
		*o_tkhost_width = tkh_w;
}

int ItemFarm::getTicketHostCount(const QString & i_name) const
{
	QMap<QString, af::Farm::Tiks>::const_iterator it = m_tickets_host.find(i_name);
	if (it != m_tickets_host.end())
		return it.value().count;

	if (m_parent_pool)
		return m_parent_pool->getTicketHostCount(i_name);

	return -1;
}

