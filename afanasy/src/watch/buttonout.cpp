#include "buttonout.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const int ButtonOut::ms_width  = 10;
const int ButtonOut::ms_height = 150;

ButtonOut * ButtonOut::ms_Buttons[LAST] = { NULL, NULL, NULL};
ButtonOut * ButtonOut::ms_Current = NULL;
int ButtonOut::ms_CurrentType = Center;

ButtonOut::ButtonOut( const int i_button_position, QWidget * i_parent):
    QWidget( i_parent),
    m_type( i_button_position)
{
    setUnSelected();

    ms_Buttons[m_type] = this;
}

ButtonOut::~ButtonOut()
{
    ms_Buttons[m_type] = NULL;
    if( ms_CurrentType == m_type)
    {
        ms_Current = NULL;
        ms_CurrentType = Center;
    }
}

void ButtonOut::mousePressEvent( QMouseEvent * i_event )
{
    if( i_event->button() != Qt::LeftButton) return;

    for( int b = 0; b < LAST; b++) if( ms_Buttons[b] ) ms_Buttons[b]->setUnSelected();

    if( ms_CurrentType != m_type )
    {
        if( !m_selected )
        {
            setSelected();
            ms_Current = this;
            ms_CurrentType = m_type;
        }
    }
    else
    {
        ms_Current = NULL;
        ms_CurrentType = Center;
    }
}

void ButtonOut::setSelected()
{
    setBackgroundRole( QPalette::Mid );
    m_selected = true;
}

void ButtonOut::setUnSelected()
{
    setBackgroundRole( QPalette::Shadow );
    m_selected = false;
}

void ButtonOut::reloadImages()
{
    switch( m_type)
    {
    case Left:
    {
        Watch::loadImage( m_img_on,  afqt::QEnvironment::image_snap_lefton.str  );
        Watch::loadImage( m_img_off, afqt::QEnvironment::image_snap_leftoff.str );
        break;
    }
    case Right:
    {
        Watch::loadImage( m_img_on,  afqt::QEnvironment::image_snap_righton.str  );
        Watch::loadImage( m_img_off, afqt::QEnvironment::image_snap_rightoff.str );
        break;
    }
    }

    int width  = ms_width;
    int height = ms_height;

    if( m_img_on.isNull() || m_img_off.isNull())
    {
        if( false == m_img_on.isNull() ) m_img_on  = QPixmap();
        if( false == m_img_off.isNull()) m_img_off = QPixmap();
        setAutoFillBackground( true);
    }
    else
    {
        setAutoFillBackground( false);
        width  = m_img_on.width();
        height = m_img_on.height();
        if( width  < m_img_off.width() ) width  = m_img_off.width();
        if( height < m_img_off.height()) height = m_img_off.height();
    }

    setFixedSize( width, height);
}

void ButtonOut::paintEvent( QPaintEvent * i_event)
{
    QPainter p( this);

    if( m_selected && ( false == m_img_on.isNull() ))
    {
        p.drawPixmap( 0, 0, m_img_on);
        return;
    }
    else if( false == m_img_off.isNull() )
    {
        p.drawPixmap( 0, 0, m_img_off);
        return;
    }

    QRect r( rect());
    int x = r.x(); int y = r.y(); int w = r.width(); int h = r.height();
    p.setPen( afqt::QEnvironment::qclr_black);
    p.drawRect( x,y,w-1,h-1);

    if( false == m_selected)  return;

    p.setFont( afqt::QEnvironment::f_info );
    static const int tlen = 5;
    static const char text[tlen] = "snap";
    for( int i = 0; i < tlen-1; i++)
    {
        int pos = (h>>3) * (i+2);
        p.drawText( x, pos-10, w, 20, Qt::AlignHCenter | Qt::AlignVCenter, QString(text[i]));
    }
}
