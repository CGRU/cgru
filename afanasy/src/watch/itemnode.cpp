#include "itemnode.h"

#include "../include/afjob.h"

#include "../libafqt/name_afqt.h"

#include "ctrlsortfilter.h"

#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

ItemNode::ItemNode( af::Node *node, const CtrlSortFilter * i_ctrl_sf):
	Item( afqt::stoq( node->getName()), node->getId()),
	m_ctrl_sf( i_ctrl_sf),
	m_sort_int1( 0),
	m_sort_int2( 0),
	m_flagshidden( 0)
{
	m_locked = node->isLocked();
}

ItemNode::~ItemNode()
{
}

void ItemNode::updateNodeValues( const af::Node * i_node)
{
	m_priority    = i_node->getPriority();
	m_annotation  = afqt::stoq( i_node->getAnnotation());
	m_custom_data = afqt::stoq( i_node->getCustomData());
}

void ItemNode::paint( QPainter *painter, const QStyleOptionViewItem &option) const
{
	Item::paint( painter, option);

	painter->drawText( option.rect, Qt::AlignVCenter | Qt::AlignHCenter, QString(" node "));
}

bool ItemNode::compare( const ItemNode & other) const
{
	bool result = false;
	bool found = false;

	// Sort by the 1st parameter:
	if(( false == m_sort_str1.isEmpty()) && ( false == other.m_sort_str1.isEmpty()))
	{
		if( m_sort_str1 != other.m_sort_str1 )
		{
			result = m_sort_str1 > other.m_sort_str1;
			found = true;
		}
	}
	else
	{
		if( m_sort_int1 != other.m_sort_int1 )
		{
			result = m_sort_int1 > other.m_sort_int1;
			found = true;
		}
	}
	if( false == m_ctrl_sf->isSortAscending1())
		result = ( false == result );

	// Sort by the 2nd parameter:
	if( false == found )
	{
		if(( false == m_sort_str2.isEmpty()) && ( false == other.m_sort_str2.isEmpty()))
		{
			if( m_sort_str2 != other.m_sort_str2 )
			{
				result = m_sort_str2 > other.m_sort_str2;
				found = true;
			}
		}
		else
		{
			if( m_sort_int2 != other.m_sort_int2 )
			{
				result = m_sort_int2 > other.m_sort_int2;
				found = true;
			}
		}

		if( false == m_ctrl_sf->isSortAscending2())
			result = ( false == result );
	}

	// Sort by the name:
	if( false == found )
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

bool ItemNode::getHidden( int32_t i_flags) const
{
	bool result = m_flagshidden & i_flags;

	if( i_flags & ListNodes::e_HideInvert)
		result = !result;

	return result;
}
