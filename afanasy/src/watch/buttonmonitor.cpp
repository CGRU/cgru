#include "buttonmonitor.h"

#include "../libafanasy/environment.h"

#include "../libafqt/qenvironment.h"

#include "wndlist.h"

#include <QtCore/QEvent>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QMenu>
#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const int ButtonMonitor::ButtonsHeight = 25;
ButtonMonitor * ButtonMonitor::Buttons[Watch::WLAST] = {0,0,0,0,0,0};
ButtonMonitor * ButtonMonitor::Current = NULL;
int ButtonMonitor::CurrentType = Watch::WNONE;

ButtonMonitor::ButtonMonitor( int wType, QWidget *parent):
   QWidget( parent),
   hovered( false),
   pressed( false),
   type( Watch::WNONE)
{
   if((wType <= Watch::WNONE) || (wType >= Watch::WLAST))
   {
      AFERRAR("ButtonMonitor::ButtonMonitor: Invalid type = %d", wType)
      return;
   }
   type = wType;
   if( Buttons[type] != NULL )
   {
      AFERRAR("ButtonMonitor::ButtonMonitor: Type = %d, already exists.", wType)
      return;
   }
   Buttons[type] = this;

   createImage();

   QString tooltip = QString("Show %1 list.").arg(Watch::WndName[wType]);
   if( wType != Watch::WJobs ) tooltip += "\nUse RMB to open new window.";
   setToolTip(tooltip);
}

void ButtonMonitor::createImage()
{
   // Calculate button images filename:
   static const QString img_extension = ".png";
   QString img_filename = afqt::stoq( af::Environment::getAfRoot())
           + "/icons/watch/" + afqt::QEnvironment::theme.str + "/buttons/";
   switch( type)
   {
      case Watch::WJobs:
         img_filename += "jobs";
         break;
      case Watch::WRenders:
         img_filename += "renders";
         break;
      case Watch::WUsers:
         img_filename += "users";
         break;
      case Watch::WTalks:
         img_filename += "talks";
         break;
      case Watch::WMonitors:
         img_filename += "monitors";
         break;
   }

   // Load button images:
   useimages = true;
   QString filename = img_filename + img_extension;
   if( img.load( filename) == false)
   {
      AFINFA("ButtonMonitor::ButtonMonitor: Unable to load '%s' image.", filename.toUtf8().data())
      useimages = false;
   }
   filename = img_filename + "_h" + img_extension;
   if( img_h.load( filename) == false)
   {
      AFINFA("ButtonMonitor::ButtonMonitor: Unable to load '%s' image.", filename.toUtf8().data())
      useimages = false;
   }
   filename = img_filename + "_p" + img_extension;
   if( img_p.load( filename) == false)
   {
      AFINFA("ButtonMonitor::ButtonMonitor: Unable to load '%s' image.", filename.toUtf8().data())
      useimages = false;
   }
   filename = img_filename + "_t" + img_extension;
   if( img_t.load( filename) == false)
   {
      AFINFA("ButtonMonitor::ButtonMonitor: Unable to load '%s' image.", filename.toUtf8().data())
      useimages = false;
   }

   if( useimages )
   {
      // Calculate images color:
      typedef union _rgba
      {
         uint8_t c[4];
         uint32_t all;
      } RGBA;
      width = img.width();
      height = img.height();
      static const int images_num = 4;
      QImage * images[images_num] = { &img, &img_h, &img_p, &img_t};
      QColor back;
      if( af::Environment::GOD())         back = afqt::QEnvironment::clr_LinkVisited.c;
      else if( af::Environment::VISOR())  back = afqt::QEnvironment::clr_Link.c;
      else                                back = afqt::QEnvironment::clr_Window.c;
      for( int i = 0; i < 4; i++)
      for( int y = 0; y < height; y++)
      for( int x = 0; x < width; x++)
      {
         QRgb qrgb = images[i]->pixel( x, y);
         RGBA rgba;
         int r = back.red();
         int g = back.green();
         int b = back.blue();
         int ri = qRed( qrgb);
         int gi = qGreen( qrgb);
         int bi = qBlue( qrgb);
         if( ri > 128 ) r += ri - 128; else r = r * ri / 128;
         if( gi > 128 ) g += gi - 128; else g = g * gi / 128;
         if( bi > 128 ) b += bi - 128; else b = b * bi / 128;
         if( r < 0 ) r = 0; if( r > 0xff ) r = 0xff;
         if( g < 0 ) g = 0; if( g > 0xff ) g = 0xff;
         if( b < 0 ) b = 0; if( b > 0xff ) b = 0xff;
         rgba.c[3] = 0xff;
         rgba.c[2] = r;
         rgba.c[1] = g;
         rgba.c[0] = b;
         images[i]->setPixel( x, y, rgba.all);
      }
   }
   else
   {
      switch( type)
      {
         case Watch::WJobs:
         case Watch::WRenders:
         case Watch::WUsers:
            width = 110;
            height = 30;
            break;
         case Watch::WTalks:
         case Watch::WMonitors:
            width = 40;
            height = 24;
      }
   }

   setFixedSize( width, height);
   repaint();
}

void ButtonMonitor::contextMenuEvent( QContextMenuEvent *event)
{
   if( Watch::isConnected() == false ) return;
   if((type == CurrentType) || (type == Watch::WJobs)) return;
   QString itemname;
   if( Watch::opened[type]) itemname = "Raise";
   else itemname = "Open";

   QMenu menu(this);
   QAction *action;
   action = new QAction( itemname, this);
   connect( action, SIGNAL( triggered() ), this, SLOT( open_SLOT() ));
   menu.addAction( action);
   menu.exec( event->globalPos());
}

ButtonMonitor::~ButtonMonitor()
{
   Buttons[type] = NULL;
   if( Current == this)
   {
      Current = NULL;
      CurrentType = Watch::WNONE;
   }
}

void ButtonMonitor::refreshImages()
{
   for( int b = 0; b < Watch::WLAST; b++) if( Buttons[b]) Buttons[b]->createImage();
}

void ButtonMonitor::open_SLOT() { openMonitor( true);}

void ButtonMonitor::openMonitor( bool inSeparateWindow)
{
   if( Watch::openMonitor( type, inSeparateWindow) && ( false == inSeparateWindow ))
   {
      unset();
      repaint();
      Current = this;
      CurrentType = type;
   }
}

void ButtonMonitor::pushButton( int wType)
{
   if( Buttons[ wType] == NULL)
   {
      AFERRAR("ButtonMonitor::pushButton: Buttons[%s] is NULL.", Watch::BtnName[wType].toUtf8().data())
      return;
   }
   Buttons[wType]->openMonitor( false);
}

void ButtonMonitor::unset()
{
   Current = NULL;
   CurrentType = Watch::WNONE;
   for( int b = 0; b < Watch::WLAST; b++) if( Buttons[b]) Buttons[b]->repaint();
}

void ButtonMonitor::enterEvent ( QEvent * event)
{
   hovered = true;
   repaint();
}
void ButtonMonitor::leaveEvent( QEvent *event)
{
   hovered = false;
   repaint();
}
void ButtonMonitor::mousePressEvent( QMouseEvent * event )
{
   if( event->button() == Qt::LeftButton)
   {
      pressed = true;
      openMonitor( false);
   }
}
void ButtonMonitor::mouseReleaseEvent( QMouseEvent * event )
{
   if( pressed == false ) return;
   pressed = false;
   repaint();
}

void ButtonMonitor::paintEvent( QPaintEvent * event )
{
   QPainter painter( this);

   if( useimages)
   {
      if( pressed )
         painter.drawImage( rect(), img_p);
      else if( type == CurrentType )
         painter.drawImage( rect(), img_t);
      else if( hovered )
         painter.drawImage( rect(), img_h);
      else
         painter.drawImage( rect(), img);
   }
   else
   {
      QString text = Watch::BtnName[type];
      if( pressed ) text = QString("[%1]").arg(text);
      else if( type == CurrentType) text = QString("[*%1*]").arg(text);
      else if( hovered ) text = QString("=%1=").arg(text);
      painter.drawText( 0, 0, width-1, height-1, Qt::AlignHCenter | Qt::AlignVCenter, text);
      painter.drawRect( 0, 0, width-1, height-1);
   }
}
