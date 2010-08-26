#include "viewitems.h"

#include <QtCore/QEvent>
#include <QtGui/QKeyEvent>

#include "item.h"
#include "watch.h"
#include "listitems.h"

ItemDelegate::ItemDelegate( QWidget *parent):
   QAbstractItemDelegate( parent)
{
}

void ItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   if( qVariantCanConvert<Item*>(index.data()))
      qVariantValue<Item*>( index.data())->paint( painter, option);
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   if( qVariantCanConvert<Item*>(index.data()))
      return qVariantValue<Item*>(index.data())->sizeHint( option);
   return QSize();
}

void ItemDelegate::emitSizeHintChanged( const QModelIndex &index)
{
#if QT_VERSION >= 0x040400
   emit sizeHintChanged( index);
#endif
}

ViewItems::ViewItems( QWidget * parent):
   QListView( parent),
   listitems( NULL)
//   mousePressEvent_handler( NULL)
{
   setSpacing( 3);
   setUniformItemSizes( false);

#if QT_VERSION >= 0x040300
   setSelectionRectVisible( true);
#endif
   setSelectionMode( QAbstractItemView::ExtendedSelection);

   viewport()->setBackgroundRole( QPalette::Mid);
   viewport()->setAutoFillBackground( true);

   itemDelegate = new ItemDelegate;
   setItemDelegate( itemDelegate);
}

ViewItems::~ViewItems()
{
}

void ViewItems::emitSizeHintChanged( const QModelIndex &index)
{
   itemDelegate->emitSizeHintChanged( index);
}

void ViewItems::keyPressEvent( QKeyEvent * event)
{
   if(( selectionMode() != QAbstractItemView::NoSelection ) && ( event->key() == Qt::Key_Escape )) clearSelection();
   QListView::keyPressEvent( event);

#if QT_VERSION >= 0x040600
   Watch::keyPressEvent( event);
#endif
}

void ViewItems::mousePressEvent( QMouseEvent * event)
{
   if( listitems)
      if( listitems->mousePressed( event))
         return;
   QListView::mousePressEvent( event);
}
