#include "labelversion.h"

#include <QtCore/QFile>
#include <QtGui/QPainter>

#include "../libafanasy/environment.h"
#include "../libafqt/qenvironment.h"

LabelVersion::LabelVersion( QWidget *parent):
   QWidget( parent)
{
   setMinimumHeight(16);
   setMaximumHeight(16);
   text = QString("AFANASY BUILD %1      CGRU VERSION %2").arg(af::Environment::getAfanasyBuildVersion()).arg( QString::fromUtf8( af::Environment::getCGRUVersion().c_str()));
   setToolTip(QString("Afanasy build sources revision: %1\nCGRU version: %2").arg(af::Environment::getAfanasyBuildVersion()).arg( QString::fromUtf8( af::Environment::getCGRUVersion().c_str())));
/*
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
      if( logo.load( filename)) logo = logo.scaled( 160, 20, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
   }
*/
   font.setPointSize( 8);
   font.setBold( true);
}

LabelVersion::~LabelVersion()
{
}

void LabelVersion::paintEvent( QPaintEvent * event)
{
   QPainter p( this);

   // Fill area:
   p.fillRect( rect(), afqt::QEnvironment::clr_Window.c);

/*   // Draw logo:
   if( false == logo.isNull())
      p.drawPixmap( 0, 0, logo);
*/
   // Draw text:
   p.setPen( afqt::QEnvironment::qclr_black);
   p.setFont( font);
   p.drawText( rect(), Qt::AlignCenter, text);
}
