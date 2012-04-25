#include "itemnode.h"

#include "../include/afjob.h"

#include "../libafqt/name_afqt.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>
#include <QtCore/QTimer>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ItemNode::ItemNode( af::Node *node):
   Item( afqt::stoq( node->getName()), node->getId()),
   height( Height),
   sort_int( 0),
	m_flagshidden( 0)
{
   locked = node->isLocked();
}

ItemNode::~ItemNode()
{
}

QSize ItemNode::sizeHint( const QStyleOptionViewItem &option) const
{
   return QSize( Width, height);
}

void ItemNode::paint( QPainter *painter, const QStyleOptionViewItem &option) const
{
   Item::paint( painter, option);

   painter->drawText( option.rect, Qt::AlignVCenter | Qt::AlignHCenter, QString(" node "));
}

bool ItemNode::compare( const ItemNode & other, int operation) const
{
   if( false == sort_str.isEmpty() )
   {
      if( other.sort_str.isEmpty())
         return false;

      switch( operation)
      {
         case GREATER:        return ((sort_str) == (other.sort_str)) ? name > other.name : ((sort_str) >  (other.sort_str));
         case GREATEREQUAL:   return ( sort_str) >= (other.sort_str);
         case SMALLER:        return ((sort_str) == (other.sort_str)) ? name < other.name : ((sort_str) <  (other.sort_str));
         case SMALLEREQUAL:   return ( sort_str) <= (other.sort_str);
         case EQUAL:          return ( sort_str) == (other.sort_str);
         case NOTEQUAL:       return ( sort_str) != (other.sort_str);
         default:
         {
            AFERRAR("ItemNode::compare: Invalid operation = %d.", operation)
            return false;
         }
      }
   }
   else
   {
      switch( operation)
      {
         case GREATER:        return ((sort_int) == (other.sort_int)) ? name > other.name : ((sort_int) >  (other.sort_int));
         case GREATEREQUAL:   return ( sort_int) >= (other.sort_int);
         case SMALLER:        return ((sort_int) == (other.sort_int)) ? name < other.name : ((sort_int) <  (other.sort_int));
         case SMALLEREQUAL:   return ( sort_int) <= (other.sort_int);
         case EQUAL:          return ( sort_int) == (other.sort_int);
         case NOTEQUAL:       return ( sort_int) != (other.sort_int);
         default:
         {
            AFERRAR("ItemNode::compare: Invalid operation = %d.", operation)
            return false;
         }
      }
   }

   return false;
}

bool ItemNode::filter( const QRegExp & regexp, const bool & filtermatch)
{
	if( filter_str.isEmpty())
		return false;

	if( filtermatch )
		return regexp.exactMatch( filter_str);

	return filter_str.contains( regexp);
}

bool ItemNode::getHidden( int32_t i_flags) const
{
	bool result = m_flagshidden & i_flags;

	if( i_flags & ListNodes::e_HideInvert)
		result = !result;

	return result;
}
