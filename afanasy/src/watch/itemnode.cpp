#include "itemnode.h"

#include "../libafqt/name_afqt.h"

#include "ctrlsortfilter.h"

#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

ItemNode::ItemNode(ListNodes * i_list_nodes, af::Node * node, EType i_type, const CtrlSortFilter * i_ctrl_sf):
	Item(afqt::stoq(node->getName()), node->getId(), i_type),
	m_list_nodes(i_list_nodes),
	m_ctrl_sf( i_ctrl_sf),
	m_sort_int1( 0),
	m_sort_int2( 0),
	m_hide_flags( 0),
	m_parent_item(NULL)
{
	m_locked = node->isLocked();
}

ItemNode::~ItemNode()
{
	for (int i = 0; i < m_child_list.size(); i++)
		m_child_list[i]->m_parent_item = NULL;

	if (m_parent_item)
		m_parent_item->m_child_list.removeAll(this);
}

void ItemNode::updateValues(af::Node * i_afnode, int i_msgType)
{
	switch (getType())
	{
		case TBranch:
		case TPool:
			// This nodes can have childs.
			// We store them in a map for a quick find for hierarchy.
			m_list_nodes->hrStoreParent(this);
			break;
		default:
			break;
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

void ItemNode::updateNodeValues( const af::Node * i_node)
{
	m_params["priority"]   = i_node->getPriority();
	m_params["annotation"] = afqt::stoq(i_node->getAnnotation());

	m_priority    = i_node->getPriority();
	m_annotation  = afqt::stoq( i_node->getAnnotation());
	m_custom_data = afqt::stoq( i_node->getCustomData());
}

void ItemNode::addChild(ItemNode * i_item)
{
	if (m_child_list.contains(i_item))
	{
		AF_ERR << "Node '" << afqt::qtos(getName()) << "' already has child '" << afqt::qtos(i_item->getName()) << "'.";
		return;
	}

	m_child_list.append(i_item);

	i_item->setParentItem(this);
}

void ItemNode::setParentItem(ItemNode * i_parent_item)
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

bool ItemNode::compare( const ItemNode & other) const
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
			switch(getType())
			{
				case TBranch:
					result = false;
					break;
				case TPool:
					result = false;
					break;
				default:
					result = true;
			}
			found = true;
		}
	}

	// Sort by the 1st parameter:
	if (false == found)
	{
		if ((false == m_sort_str1.isEmpty()) && (false == other.m_sort_str1.isEmpty()))
		{
			if(m_sort_str1 != other.m_sort_str1)
			{
				result = m_sort_str1 > other.m_sort_str1;
				found = true;
			}
		}
		else
		{
			if(m_sort_int1 != other.m_sort_int1)
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

	if( false == m_ctrl_sf->isFilterEnabled())
		return hide;

	if( m_ctrl_sf->isFilterEmpty())
		return hide;

	return ( false == m_ctrl_sf->getFilterRE().match( m_filter_str));
}

bool ItemNode::getHideFlags(int32_t i_hide_flags) const
{
	bool result = m_hide_flags & i_hide_flags;

	if (i_hide_flags & ListNodes::e_HideInvert)
		result = !result;

	return result;
}
