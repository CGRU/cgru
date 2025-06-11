#include "itemnode.h"

#include "../libafqt/name_afqt.h"

#include "ctrlsortfilter.h"
#include "itembutton.h"
#include "watch.h"

#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

ItemNode::ItemNode(ListNodes *i_list_nodes, af::Node *node, EType i_type, const CtrlSortFilter *i_ctrl_sf)
	: Item(afqt::stoq(node->getName()), node->getId(), i_type), m_list_nodes(i_list_nodes),
	  m_ctrl_sf(i_ctrl_sf), m_sort_int1(0), m_sort_int2(0), m_hide_flags(0), m_childs_hidden(false),
	  m_btn_childs_hide(NULL), m_btn_childs_show(NULL), m_parent_item(NULL)
{
	m_locked = node->isLocked();
}

void ItemNode::addChildsHideShowButton()
{
	m_btn_childs_hide = new ItemButton("hide_childs", 2, 2, 12, "▼", "Hide childs.");
	m_btn_childs_hide->setHidden(true);
	m_btn_childs_show = new ItemButton("show_childs", 2, 2, 12, "▶", "Show childs.");
	m_btn_childs_show->setHidden(true);

	addButton(m_btn_childs_hide);
	addButton(m_btn_childs_show);
}

ItemNode::~ItemNode()
{
	for (int i = 0; i < m_child_list.size(); i++)
		m_child_list[i]->m_parent_item = NULL;

	if (m_parent_item)
	{
		m_parent_item->m_child_list.removeAll(this);
		m_parent_item->showHideChildsShowHideButtons();
	}
}

void ItemNode::updateValues(af::Node *i_afnode, int i_msgType)
{
	switch (getType())
	{
		case TBranch:
		case TPool:
			// This nodes can have childs.
			// We store them in a map for a quick find for hierarchy.
			m_list_nodes->hrStoreParent(this);
			break;
		default: break;
	}

	// Store parent path before update.
	QString old_parent_path = getParentPath();

	v_updateValues(i_afnode, i_msgType);

	if (old_parent_path != getParentPath())
	{
		// Node parent path was changed.
		m_list_nodes->hrParentChanged(this);
	}

	// We can't call updateNodeValues here, as there can be only specific node information.
	// For example renders resources message does not contain priority and annotation.
}

void ItemNode::updateNodeValues(const af::Node *i_node)
{
	m_params["priority"] = i_node->getPriority();
	m_params["annotation"] = afqt::stoq(i_node->getAnnotation());

	m_priority = i_node->getPriority();
	m_annotation = afqt::stoq(i_node->getAnnotation());
	m_custom_data = afqt::stoq(i_node->getCustomData());

	m_running_services.clear();
	for (auto const &it : i_node->getRunnigServices())
		m_running_services[afqt::stoq(it.first)] = it.second;
}

void ItemNode::updateStrParameters(QString &o_str)
{
	if (Watch::isPadawan())
	{
		o_str += QString(" Priority:%1").arg(m_priority);
	}
	else if (Watch::isJedi())
	{
		o_str += QString(" Pri:%1").arg(m_priority);
	}
	else
	{
		o_str += QString(" p%1").arg(m_priority);
	}

	if (m_custom_data.size())
		o_str += " CD*";
}

void ItemNode::updateInfo()
{
	if (m_running_services.size())
	{
		m_info_text += "<br>Running services:";
		QMapIterator<QString, int> it(m_running_services);
		while (it.hasNext())
		{
			it.next();
			m_info_text += QString("<br><b>%1: %2</b>").arg(it.key()).arg(it.value());
		}
	}

	if (m_custom_data.size())
		m_info_text += QString("<br><br><i><b>Custom Data</b>:<br>%1</i>").arg(m_custom_data);
}

void ItemNode::addChild(ItemNode *i_item)
{
	if (m_child_list.contains(i_item))
	{
		AF_ERR << "Node '" << afqt::qtos(getName()) << "' already has child '"
			   << afqt::qtos(i_item->getName()) << "'.";
		return;
	}

	m_child_list.append(i_item);

	i_item->setParentItem(this);

	showHideChildsShowHideButtons();
}

void ItemNode::setParentItem(ItemNode *i_parent_item)
{
	if (m_parent_item)
		m_parent_item->m_child_list.removeAll(this);

	m_parent_item = i_parent_item;

	int depth = 0;
	if (m_parent_item)
		depth = m_parent_item->getDepth() + 1;

	setDepth(depth);

	v_parentItemChanged();
}

void ItemNode::v_parentItemChanged() {}

bool ItemNode::compare(const ItemNode &other) const
{
	bool result = false;
	bool found = false;

	// Sorting may be forced for hierarchy (branch, pool):
	if ((false == m_sort_force.isEmpty()) && (false == other.m_sort_force.isEmpty()))
	{
		if (m_sort_force != other.m_sort_force)
		{
			result = m_sort_force > other.m_sort_force;
			found = true;
		}
		else if (getType() != other.getType())
		{
			// If path is the same we should show parent node first:
			switch (getType())
			{
				case TBranch: result = false; break;
				case TPool: result = false; break;
				default: result = true;
			}
			found = true;
		}
	}

	// Sort by the 1st parameter:
	if (false == found)
	{
		if ((false == m_sort_str1.isEmpty()) && (false == other.m_sort_str1.isEmpty()))
		{
			if (m_sort_str1 != other.m_sort_str1)
			{
				result = m_sort_str1 > other.m_sort_str1;
				found = true;
			}
		}
		else
		{
			if (m_sort_int1 != other.m_sort_int1)
			{
				result = m_sort_int1 > other.m_sort_int1;
				found = true;
			}
		}

		if (false == m_ctrl_sf->isSortAscending1())
			result = (false == result);
	}

	// Sort by the 2nd parameter:
	if (false == found)
	{
		if ((false == m_sort_str2.isEmpty()) && (false == other.m_sort_str2.isEmpty()))
		{
			if (m_sort_str2 != other.m_sort_str2)
			{
				result = m_sort_str2 > other.m_sort_str2;
				found = true;
			}
		}
		else
		{
			if (m_sort_int2 != other.m_sort_int2)
			{
				result = m_sort_int2 > other.m_sort_int2;
				found = true;
			}
		}

		if (false == m_ctrl_sf->isSortAscending2())
			result = (false == result);
	}

	// Sort by the name:
	if (false == found)
		result = m_name > other.m_name;

	return result;
}

bool ItemNode::filter()
{
	bool hide = false;

	switch (getType())
	case TPool:
	case TBranch:
		return hide;

	if (false == m_ctrl_sf->isFilterEnabled())
		return hide;

	if (m_ctrl_sf->isFilterEmpty())
		return hide;

	return (false == m_ctrl_sf->getFilterRE().match(m_filter_str));
}

bool ItemNode::getHideFlags(int32_t i_hide_flags) const
{
	bool result = m_hide_flags & i_hide_flags;

	if (i_hide_flags & ListNodes::e_HideInvert)
		result = !result;

	return result;
}

bool ItemNode::isHiddenByParents() const
{
	if (NULL == m_parent_item)
		return false;

	if (m_parent_item->m_childs_hidden)
		return true;

	return m_parent_item->isHiddenByParents();
}

void ItemNode::setChildsHidden(bool i_hidden)
{
	if (i_hidden == m_childs_hidden)
		return;

	m_childs_hidden = i_hidden;

	m_list_nodes->processHidden();
}

void ItemNode::v_buttonClicked(ItemButton *i_b)
{
	if ((m_btn_childs_hide && m_btn_childs_show) &&
		((i_b == m_btn_childs_hide) || (i_b == m_btn_childs_show)))
	{
		if (i_b == m_btn_childs_hide)
		{
			setChildsHidden(true);
		}
		else if (i_b == m_btn_childs_show)
		{
			setChildsHidden(false);
		}

		m_btn_childs_hide->setHidden(m_childs_hidden);
		m_btn_childs_show->setHidden(false == m_childs_hidden);
	}
}

void ItemNode::showHideChildsShowHideButtons()
{
	// May be show/hide childs buttons were not created
	if (NULL == m_btn_childs_hide)
		return;

	if (m_child_list.size())
	{
		m_btn_childs_hide->setHidden(m_childs_hidden);
		m_btn_childs_show->setHidden(false == m_childs_hidden);
	}
	else
	{
		m_btn_childs_hide->setHidden(true);
		m_btn_childs_show->setHidden(true);
	}
}

void ItemNode::drawRunningServices(QPainter *i_painter, int i_x, int i_y, int i_w, int i_h) const
{
	if (m_running_services.size() == 0)
		return;

	// Calculate icons total count
	int total_count = 0;
	{
		QMapIterator<QString, int> it(m_running_services);
		while (it.hasNext())
		{
			it.next();
			total_count += it.value();
		}
	}

	static const int icons_size = 16;
	static const int dx_max = icons_size + 5;

	// Calculate distance between each icon (delta x)
	int dx = dx_max;
	if (total_count > m_running_services.size())
	{
		dx = (i_w - m_running_services.size() * dx_max) / (total_count - m_running_services.size());
	}
	// Clamp dx
	if (dx < 2)
	{
		dx = 2;
		total_count = i_w / 2 - (m_running_services.size() * dx_max);
	}
	else if (dx > dx_max)
		dx = dx_max;

	// Calculate x_offset, due maximum distance between icons
	const int total_width =
		(total_count - m_running_services.size()) * dx + m_running_services.size() * dx_max;
	const int x_offset = (i_w - total_width) / 2;

	// Draw
	int x = i_x + x_offset;
	QMapIterator<QString, int> it(m_running_services);
	while (it.hasNext())
	{
		it.next();

		for (int i = 0; i < it.value(); i++)
		{
			const QPixmap *icon = Watch::getServiceIconSmall(it.key());
			if (icon)
				i_painter->drawPixmap(x, i_y, *icon);
			else
				drawStar(icons_size / 2, x, i_y + icons_size / 2, i_painter);

			x += dx;
		}

		// Show service icon full
		x = x - dx + dx_max;
	}
	/*
		// Draw total count text
		QString running_str = QString("T:%1").arg(total_count);
		i_painter->setFont(afqt::QEnvironment::f_name);
		i_painter->setPen(afqt::QEnvironment::clr_textstars.c);
		i_painter->drawText(i_x, i_y, i_w, i_h, Qt::AlignHCenter | Qt::AlignBottom, running_str);
	*/
	/*
		// Draw border rectangle
		i_painter->setPen(afqt::QEnvironment::clr_outline.c);
		i_painter->setBrush(Qt::NoBrush);
		i_painter->drawRect(i_x, i_y, i_w, i_h);
	*/
}
