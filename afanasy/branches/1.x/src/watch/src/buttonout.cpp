#include "buttonout.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>

#include <qenvironment.h>

//#define AFOUTPUT
////#undef AFOUTPUT
#include <macrooutput.h>

const int ButtonOut::width  = 150;
const int ButtonOut::height =  10;

ButtonOut * ButtonOut::buttons[LAST] = { 0, 0, 0, 0, 0};
ButtonOut * ButtonOut::Current = NULL;
int ButtonOut::CurrentType = Center;

ButtonOut::ButtonOut( const int button_position, QWidget *parent):
   QWidget( parent),
   type( button_position)
{
   setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed);
   setAutoFillBackground( true);
   setUnSelected();

   switch( type)
   {
   case Left:
   {
      setFixedSize( height, width);
      break;
   }
   case Right:
   {
      setFixedSize( height, width);
      break;
   }
   case Top:
   {
      setFixedSize( width, height);
      break;
   }
   case Bot:
   {
      setFixedSize( width, height);
      break;
   }
   case Center:
   default:
      AFERRAR("ButtonOut::ButtonOut: Invalid type = %d", type);
   }

   buttons[type] = this;
}

ButtonOut::~ButtonOut()
{
   buttons[type] = NULL;
   if( CurrentType == type)
   {
      Current = NULL;
      CurrentType = Center;
   }
}

void ButtonOut::mousePressEvent( QMouseEvent * event )
{
   if( event->button() != Qt::LeftButton) return;

   for( int b = 0; b < LAST; b++) if( buttons[b] ) buttons[b]->setUnSelected();

   if( CurrentType != type )
   {
      if( !selected )
      {
         setSelected();
         Current = this;
         CurrentType = type;
      }
   }
   else
   {
      Current = NULL;
      CurrentType = Center;
   }
}

void ButtonOut::setSelected()
{
   setBackgroundRole( QPalette::Mid );
   selected = true;
}

void ButtonOut::setUnSelected()
{
   setBackgroundRole( QPalette::Shadow );
   selected = false;
}

void ButtonOut::paintEvent( QPaintEvent * event)
{
   QPainter painter( this);
   QRect r( rect());
   int x = r.x(); int y = r.y(); int w = r.width(); int h = r.height();
   painter.setPen( afqt::QEnvironment::qclr_black);
   painter.drawRect( x,y,w-1,h-1);

   if( false == selected)  return;

   painter.setFont( afqt::QEnvironment::f_info );
   static const int tlen = 5;
   static const char text[tlen] = "snap";
   for( int i = 0; i < tlen-1; i++)
   {
      int pos = (h>>3) * (i+2);
      painter.drawText( x, pos-10, w, 20, Qt::AlignHCenter | Qt::AlignVCenter, QString(text[i]));
   }
}
