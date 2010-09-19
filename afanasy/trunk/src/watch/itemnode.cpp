#include "itemnode.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>
#include <QtCore/QTimer>

#include "../include/afjob.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ItemNode::ItemNode( af::Node *node):
   Item( node->getName(), node->getId()),
   height( Height),
   sort_int( NULL),
   sort_uint( NULL),
   sort_str( NULL),
   filter_str( NULL)
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
   if( sort_int )
   {
      if( other.sort_int == NULL )
      {
         AFERROR("ItemNode::compare: other.sort_int == NULL.\n");
         return false;
      }
      switch( operation)
      {
         case GREATER:        return ((*sort_int) == (*other.sort_int)) ? name > other.name : ((*sort_int) >  (*other.sort_int));
         case GREATEREQUAL:   return ( *sort_int) >= (*other.sort_int);
         case SMALLER:        return ((*sort_int) == (*other.sort_int)) ? name < other.name : ((*sort_int) <  (*other.sort_int));
         case SMALLEREQUAL:   return ( *sort_int) <= (*other.sort_int);
         case EQUAL:          return ( *sort_int) == (*other.sort_int);
         case NOTEQUAL:       return ( *sort_int) != (*other.sort_int);
         default:
         {
            AFERRAR("ItemNode::compare: Invalid operation = %d.\n", operation);
            return false;
         }
      }
   }
   if( sort_uint )
   {
      if( other.sort_uint == NULL )
      {
         AFERROR("ItemNode::compare: other.sort_uint == NULL.\n");
         return false;
      }
      switch( operation)
      {
         case GREATER:        return ((*sort_uint) == (*other.sort_uint)) ? name > other.name : ((*sort_uint) >  (*other.sort_uint));
         case GREATEREQUAL:   return ( *sort_uint) >= (*other.sort_uint);
         case SMALLER:        return ((*sort_uint) == (*other.sort_uint)) ? name < other.name : ((*sort_uint) <  (*other.sort_uint));
         case SMALLEREQUAL:   return ( *sort_uint) <= (*other.sort_uint);
         case EQUAL:          return ( *sort_uint) == (*other.sort_uint);
         case NOTEQUAL:       return ( *sort_uint) != (*other.sort_uint);
         default:
         {
            AFERRAR("ItemNode::compare: Invalid operation = %d.\n", operation);
            return false;
         }
      }
   }
   if( sort_str )
   {
      if( other.sort_str == NULL )
      {
         AFERROR("ItemNode::compare: other.sort_str == NULL.\n");
         return false;
      }
      switch( operation)
      {
         case GREATER:        return ((*sort_str) == (*other.sort_str)) ? name > other.name : ((*sort_str) >  (*other.sort_str));
         case GREATEREQUAL:   return ( *sort_str) >= (*other.sort_str);
         case SMALLER:        return ((*sort_str) == (*other.sort_str)) ? name < other.name : ((*sort_str) <  (*other.sort_str));
         case SMALLEREQUAL:   return ( *sort_str) <= (*other.sort_str);
         case EQUAL:          return ( *sort_str) == (*other.sort_str);
         case NOTEQUAL:       return ( *sort_str) != (*other.sort_str);
         default:
         {
            AFERRAR("ItemNode::compare: Invalid operation = %d.\n", operation);
            return false;
         }
      }
   }
   AFERROR("ItemNode::compare: Comparison vales pointers are NULL.\n");
   return false;
}

bool ItemNode::filter( const QRegExp & regexp, bool filtermatch)
{
   if( filter_str == NULL)
   {
      AFERROR("ItemNode::filter: Filter string is NULL.\n");
      return false;
   }
   if( filtermatch )
      return regexp.exactMatch( *filter_str);
   return filter_str->contains( regexp);
}
