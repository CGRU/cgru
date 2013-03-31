#include "item.h"

#define _USE_MATH_DEFINES	 
#include <math.h>

#include "../include/afjob.h"

#include "../libafqt/qenvironment.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>
#include <QtCore/QTimer>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

QPolygonF Item::ms_star_pointsInit;
QPolygonF Item::ms_star_pointsDraw;

const int Item::Height = 14;
const int Item::Width  = 100;

Item::Item( const QString &itemname, int itemid):
    name( itemname),
    locked( false),
    running( false),
    id( itemid)
{
}

Item::~Item()
{
}

QSize Item::sizeHint( const QStyleOptionViewItem &option) const
{
    return QSize( Width, Height);
}

bool Item::calcHeight()
{
    return true;
}

const QColor & Item::clrTextMain( const QStyleOptionViewItem &option) const
{
    if( locked ) return afqt::QEnvironment::clr_textbright.c;
    return afqt::QEnvironment::qclr_black;
}
const QColor & Item::clrTextInfo( const QStyleOptionViewItem &option) const
{
    if( locked ) return afqt::QEnvironment::qclr_black;
    if( running == false ) return afqt::QEnvironment::qclr_black;
    if( option.state & QStyle::State_Selected ) return afqt::QEnvironment::qclr_black;
    return afqt::QEnvironment::clr_textbright.c;
}
const QColor & Item::clrTextInfo( bool running, bool selected, bool locked)
{
    if( locked ) return afqt::QEnvironment::qclr_black;
    if( running == false ) return afqt::QEnvironment::qclr_black;
    if( selected ) return afqt::QEnvironment::qclr_black;
    return afqt::QEnvironment::clr_textbright.c;
}
const QColor & Item::clrTextMuted( const QStyleOptionViewItem &option) const
{
    if( locked ) return afqt::QEnvironment::qclr_black;
    if( option.state & QStyle::State_Selected ) return afqt::QEnvironment::clr_textbright.c;
    return afqt::QEnvironment::clr_textmuted.c;
}
const QColor & Item::clrTextDone( const QStyleOptionViewItem &option) const
{
    if( locked ) return afqt::QEnvironment::clr_textmuted.c;
    if( option.state & QStyle::State_Selected ) return afqt::QEnvironment::qclr_black;
    return afqt::QEnvironment::clr_textdone.c;
}
const QColor & Item::clrTextState( const QStyleOptionViewItem &option, bool on ) const
{
    if(on) return (option.state & QStyle::State_Selected) ? afqt::QEnvironment::clr_textmuted.c  : afqt::QEnvironment::clr_textbright.c;
    else   return (option.state & QStyle::State_Selected) ? afqt::QEnvironment::clr_textbright.c : afqt::QEnvironment::clr_textmuted.c;
}

void Item::drawBack( QPainter *painter, const QStyleOptionViewItem &option) const
{
    painter->setOpacity( 1.0);
    painter->setRenderHint( QPainter::Antialiasing, false);

    if( option.state & QStyle::State_Selected )
        painter->fillRect( option.rect, afqt::QEnvironment::clr_selected.c);
    else
        painter->fillRect( option.rect, afqt::QEnvironment::clr_item.c);
}

void Item::drawPost( QPainter *painter, const QStyleOptionViewItem &option, float alpha) const
{
    painter->setRenderHint( QPainter::Antialiasing, false);

    int x = option.rect.x();
    int y = option.rect.y();
    int w = option.rect.width();
    int h = option.rect.height();

    painter->setPen( afqt::QEnvironment::qclr_black );
    painter->setOpacity( 0.7 * alpha);

    painter->drawLine( x, y+h, x+w-1, y+h);

    painter->setOpacity( 0.2 * alpha);

    painter->drawLine( x, y+1, x, y+h-1);
    painter->drawLine( x+w-1, y+1, x+w-1, y+h-1);

    painter->setPen( afqt::QEnvironment::qclr_white );
    painter->setOpacity( 0.5 * alpha);

    painter->drawLine( x, y, x+w-1, y);

    painter->setOpacity( 1.0);
}

void Item::paint( QPainter *painter, const QStyleOptionViewItem &option) const
{
    drawBack( painter, option);

    painter->setPen( afqt::QEnvironment::qclr_black );

    painter->setFont( afqt::QEnvironment::f_name);
    painter->drawText( option.rect, Qt::AlignTop | Qt::AlignLeft, name);

    painter->setFont( afqt::QEnvironment::f_info);
    painter->drawText( option.rect, Qt::AlignBottom | Qt::AlignRight, QString(" ( virtual Item painting ) "));
}

void Item::printfState( const uint32_t state, int posx, int posy, QPainter * painter, const QStyleOptionViewItem &option) const
{
    static const int posx_d = 18;

    painter->setFont( afqt::QEnvironment::f_min);

    painter->setPen( clrTextState( option, state & AFJOB::STATE_READY_MASK));
    painter->drawText( posx, posy, AFJOB::STATE_READY_NAME_S); posx+=posx_d;

    painter->setPen( clrTextState( option, state & AFJOB::STATE_RUNNING_MASK));
    painter->drawText( posx, posy, AFJOB::STATE_RUNNING_NAME_S); posx+=posx_d;

    painter->setPen( clrTextState( option, state & AFJOB::STATE_DONE_MASK));
    painter->drawText( posx, posy, AFJOB::STATE_DONE_NAME_S); posx+=posx_d;

    painter->setPen( clrTextState( option, state & AFJOB::STATE_ERROR_MASK));
    painter->drawText( posx, posy, AFJOB::STATE_ERROR_NAME_S); posx+=posx_d;

    painter->setPen( clrTextState( option, state & AFJOB::STATE_SKIPPED_MASK));
    painter->drawText( posx, posy, AFJOB::STATE_SKIPPED_NAME_S); posx+=posx_d;

    painter->setPen( clrTextState( option, state & AFJOB::STATE_WAITDEP_MASK));
    painter->drawText( posx, posy, AFJOB::STATE_WAITDEP_NAME_S); posx+=posx_d;

    painter->setPen( clrTextState( option, state & AFJOB::STATE_WAITTIME_MASK));
    painter->drawText( posx, posy, AFJOB::STATE_WAITTIME_NAME_S); posx+=posx_d;

    painter->setPen( clrTextState( option, state & AFJOB::STATE_OFFLINE_MASK));
    painter->drawText( posx, posy, AFJOB::STATE_OFFLINE_NAME_S); posx+=posx_d;
}

void Item::drawPercent
(
   QPainter * painter,
   int posx, int posy, int width, int height,
   int whole,
   int part_a, int part_b, int part_c,
   bool drawBorder
)
{
   int x = posx;
   int y = posy;
   int h = height;

   painter->setPen( Qt::NoPen );

   if( part_a)
   {
      int w = width*part_a/whole;
      painter->setBrush( QBrush( afqt::QEnvironment::clr_done.c, Qt::SolidPattern ));
      painter->drawRect( x, y, w, h);
      x += w;
   }
   if( part_b)
   {
      int w = width*part_b/whole;
      painter->setBrush( QBrush( afqt::QEnvironment::clr_error.c, Qt::SolidPattern ));
      painter->drawRect( x, y, w, h);
      x += w;
   }
   if( part_c)
   {
      int w = width*part_c/whole;
      painter->setBrush( QBrush( afqt::QEnvironment::clr_running.c, Qt::SolidPattern ));
      painter->drawRect( x, y, w, h);
      x += w;
   }
/*
   if( width-x > 0)
   {
      painter->setBrush( QBrush( afqt::QEnvironment::clr_item.c, Qt::SolidPattern ));
      painter->drawRect( x, y, width-x, h);
   }
*/
   if( drawBorder)
   {
      painter->setPen( afqt::QEnvironment::clr_outline.c );
      painter->setBrush( Qt::NoBrush);
      painter->drawRect( posx, posy, width, height);
   }
}

void Item::calcutaleStarPoints()
{
    int numpoints = afqt::QEnvironment::star_numpoints.n * 2;
    float r_out = float( afqt::QEnvironment::star_radiusout.n) / 100.0f;
    float r_in  = float( afqt::QEnvironment::star_radiusin.n ) / 100.0f;
    float angle = float( afqt::QEnvironment::star_rotate.n);

    angle = ( 180.0 - angle ) / 360*M_PI;
    float angle_delta = float( 2 *M_PI / float( numpoints));

    ms_star_pointsInit.resize( numpoints);
    ms_star_pointsDraw.resize( numpoints);

    for( int i = 0; i < numpoints ; i++)
    {
        ms_star_pointsInit[i].setX( cosf( angle) * r_out);
        ms_star_pointsInit[i].setY(-sinf( angle) * r_out);
        i++;
        angle += angle_delta;
        ms_star_pointsInit[i].setX( cosf( angle) * r_in);
        ms_star_pointsInit[i].setY(-sinf( angle) * r_in);
        angle += angle_delta;
    }
}

void Item::drawStar( int size, int posx, int posy, QPainter * painter)
{
    painter->setRenderHint( QPainter::Antialiasing, true);

    for( int i = 0; i < ms_star_pointsInit.size(); i++)
    {
        ms_star_pointsDraw[i].setX( ms_star_pointsInit[i].x()*size + posx);
        ms_star_pointsDraw[i].setY( ms_star_pointsInit[i].y()*size + posy);
    }
    painter->setPen( afqt::QEnvironment::clr_starline.c );
    painter->setBrush( QBrush( afqt::QEnvironment::clr_star.c, Qt::SolidPattern ));
    painter->drawPolygon( ms_star_pointsDraw);//, Qt::WindingFill);
}
