#include "viewitems.h"

#include <QtCore/QEvent>
#include <QtCore/QFile>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtGui/QScrollBar>

#include "../libafanasy/environment.h"
#include "../libafqt/qenvironment.h"

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
{
   setSpacing( 3);
   setUniformItemSizes( false);

#if QT_VERSION >= 0x040300
   setSelectionRectVisible( true);
#endif
   setSelectionMode( QAbstractItemView::ExtendedSelection);

//   viewport()->setBackgroundRole( QPalette::Mid);
//   viewport()->setAutoFillBackground( true);

   itemDelegate = new ItemDelegate;
   setItemDelegate( itemDelegate);

   // Try to load user's custom logo
   QString filename = af::Environment::getHomeAfanasy() + "/logo.png";
//printf("Logo = '%s'\n", filename.toUtf8().data());
   bool custom_logo_loaded = false;
   if( QFile::exists( filename))
      if( logo.load( filename))
         custom_logo_loaded = true;

   // Load standart logo and transform it randomly
   if( false == custom_logo_loaded)
   {
      filename = af::Environment::getAfRoot() + "/icons/afanasy.png";
//printf("Logo = '%s'\n", filename.toUtf8().data());
      if( logo.load( filename))
      {
         QTransform xform;
         xform.rotate( -5.0f - 15.0f * float( rand())/RAND_MAX);
         xform.scale( .4, .4);
         logo = logo.transformed( xform, Qt::SmoothTransformation);
         logo_offset.setX( 10 + int(10 * float( rand())/RAND_MAX));
         logo_offset.setY( 10 + int(10 * float( rand())/RAND_MAX));
      }
   }
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

void ViewItems::paintEvent( QPaintEvent * event)
{
   QPainter p( viewport());

   // Fill area:
   p.fillRect( rect(), afqt::QEnvironment::clr_Mid.c);

   // Draw logo:
   if( false == verticalScrollBar()->isVisible())
      if( false == logo.isNull())
         p.drawPixmap( rect().bottomRight() - logo.rect().bottomRight() - logo_offset, logo);

   // Process drawing:
   QListView::paintEvent( event);
}
