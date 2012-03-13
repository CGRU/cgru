#include "viewitems.h"

#include <QtCore/QEvent>
#include <QtGui/QKeyEvent>

#include "item.h"

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
   emit sizeHintChanged( index);
}

ViewItems::ViewItems( QWidget * parent):
   QListView( parent)
{
   setSpacing( 3);
   setUniformItemSizes( false);

   setSelectionRectVisible( true);
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
}
