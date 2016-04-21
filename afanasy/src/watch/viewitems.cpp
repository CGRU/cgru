#include "viewitems.h"

#include "item.h"
#include "watch.h"
#include "listitems.h"

#include <QtCore/QEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QScrollBar>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ItemDelegate::ItemDelegate( QWidget *parent):
    QAbstractItemDelegate( parent)
{
}

void ItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if( Item::isItemP(index.data()))
        Item::toItemP( index.data())->paint( painter, option);
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if( Item::isItemP(index.data()))
        return Item::toItemP(index.data())->sizeHint( option);
    return QSize();
}

void ItemDelegate::emitSizeHintChanged( const QModelIndex &index)
{
    #if QT_VERSION >= 0x040400
    emit sizeHintChanged( index);
    #endif
}

ViewItems::ViewItems( ListItems * parent):
	QListView( parent),
	m_listitems( parent)
{
    setSpacing( 2);
    setUniformItemSizes( false);

    setHorizontalScrollMode(ScrollPerPixel);
    setVerticalScrollMode(ScrollPerPixel);

    #if QT_VERSION >= 0x040300
    setSelectionRectVisible( true);
    #endif
    setSelectionMode( QAbstractItemView::ExtendedSelection);

    viewport()->setBackgroundRole( QPalette::Mid);
    viewport()->setAutoFillBackground( true);

    loadImage();

    itemDelegate = new ItemDelegate;
    setItemDelegate( itemDelegate);
}

void ViewItems::loadImage()
{
#ifdef DRAWBACK
AFINFA("ViewItems::loadImage: '%s'", afqt::QEnvironment::image_back.str.toUtf8().data());
if( afqt::QEnvironment::image_back.str.isEmpty() && ( false == m_back_filename.isEmpty()))
{
    m_back_filename.clear();
    m_back_pixmap = QPixmap();
}
else
{
    m_back_filename = afqt::QEnvironment::image_back.str;

    if( false == m_back_pixmap.load( m_back_filename) )
    {
        if( false == m_back_pixmap.load( afqt::stoq(af::Environment::getAfRoot())
                            + "/icons/watch/"
                            + afqt::QEnvironment::theme.str + "/"
                            + m_back_filename))
            m_back_pixmap = QPixmap();
    }

    if( false == m_back_pixmap.isNull() )
    {
        m_back_angle = int( 20.0f * rand() / RAND_MAX );
        m_back_offset_x = int( 20.0f * rand() / RAND_MAX );
        m_back_offset_y = int( 20.0f * rand() / RAND_MAX );
        m_back_pixmap = m_back_pixmap.transformed(
            QTransform().rotate( m_back_angle ), Qt::SmoothTransformation);
    }
}
#endif
}

ViewItems::~ViewItems()
{
    if( itemDelegate ) delete itemDelegate;
}

void ViewItems::emitSizeHintChanged( const QModelIndex &index)
{
    itemDelegate->emitSizeHintChanged( index);
}

void ViewItems::keyPressEvent( QKeyEvent * event)
{
	// Clear selection on Escape:
	if(( selectionMode() != QAbstractItemView::NoSelection ) && ( event->key() == Qt::Key_Escape ))
		clearSelection();

	// Process List view keys:
	QListView::keyPressEvent( event);

	// Process dialog keys (for admin mode):
	Watch::keyPressEvent( event);

	// Process parent (ListItems class) keys (panel buttons):
	m_listitems->keyPressEvent( event);
}

void ViewItems::mousePressEvent( QMouseEvent * event)
{
	if( m_listitems->mousePressed( event))
		return;

	QListView::mousePressEvent( event);
}

void ViewItems::repaintViewport()
{
    viewport()->repaint();
}

#ifdef DRAWBACK
void ViewItems::paintEvent( QPaintEvent * event )
{
    AFINFO("ViewItems::paintEvent:");
    if( m_back_filename != afqt::QEnvironment::image_back.str )
        loadImage();

    if( m_back_pixmap.isNull() ||
        verticalScrollBar()->isVisible())
    {
        QListView::paintEvent( event );
        return;
    }

    QPainter p( viewport());

    QRect r = viewport()->rect();
    int x = r.width();
    int y = r.height();
    int w = m_back_pixmap.width();
    int h = m_back_pixmap.height();

    x -= w + m_back_offset_x;
    y -= h + m_back_offset_y;

    p.drawPixmap( x, y, w, h, m_back_pixmap);

    QListView::paintEvent( event );
}
#endif
