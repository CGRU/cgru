#include "item.h"

#include <afjob.h>

#include <QtCore/QEvent>
#include <QtGui/QPainter>
#include <QtCore/QTimer>

#include <qenvironment.h>

//#define AFOUTPUT
////#undef AFOUTPUT
#include <macrooutput.h>

QPointF Item::star_pointsInit[10] = { QPointF(), QPointF(), QPointF(), QPointF(), QPointF(), QPointF(), QPointF(), QPointF(), QPointF(), QPointF()};
QPointF Item::star_pointsDraw[10] = { QPointF(), QPointF(), QPointF(), QPointF(), QPointF(), QPointF(), QPointF(), QPointF(), QPointF(), QPointF()};

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
   painter->setRenderHint( QPainter::Antialiasing, false);

   if( option.state & QStyle::State_Selected )
      painter->fillRect( option.rect, afqt::QEnvironment::clr_selected.c);
   else
      painter->fillRect( option.rect, afqt::QEnvironment::clr_item.c);
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
   static const int posx_d = 15;

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

void Item::drawStar( int size, int posx, int posy, QPainter * painter)
{
   painter->setRenderHint( QPainter::Antialiasing, true);

   for( int i = 0; i < 10; i++)
   {
      star_pointsDraw[i].setX( star_pointsInit[i].x()*size + posx);
      star_pointsDraw[i].setY( star_pointsInit[i].y()*size + posy);
   }
   painter->setPen( afqt::QEnvironment::clr_starline.c );
   painter->setBrush( QBrush( afqt::QEnvironment::clr_star.c, Qt::SolidPattern ));
   painter->drawPolygon( star_pointsDraw, 10);//, Qt::WindingFill);
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
      painter->setBrush( QBrush( afqt::QEnvironment::clr_running.c, Qt::SolidPattern ));
      painter->drawRect( x, y, w, h);
      x += w;
   }
   if( part_c)
   {
      int w = width*part_c/whole;
      painter->setBrush( QBrush( afqt::QEnvironment::clr_error.c, Qt::SolidPattern ));
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
