#include "labelversion.h"

#include "watch.h"
#include "wndtext.h"

#include <QtCore/QFile>
#include <QtGui/QPainter>
#include <QtGui/QTextEdit>

LabelVersion::LabelVersion( QWidget *parent):
    QWidget( parent),
    status( SS_None)
{
    //setSizePolicy( QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum));
    setMinimumHeight(16);
    setMaximumHeight(16);

    label = QString("CGRU VERSION %1").arg(af::Environment::getVersionCGRU().c_str());
    tooltip = QString("Build at %1"
                     "\nRevision: %2"
                     "\nPython version: %3"
                     "\nQt version: %4")
         .arg( af::Environment::getVersionDate().c_str())
         .arg( af::Environment::getVersionRevision().c_str())
         .arg( af::Environment::getVersionPython().c_str())
         .arg( qVersion());
    if( false == af::Environment::getVersionGCC().empty())
        tooltip += QString("\nGCC version: %1").arg( af::Environment::getVersionGCC().c_str());

    tooltip += QString("\nCGRU_LOCATION=%1"
                      "\nAFANASY:"
                      "\nAF_ROOT=%2"
                      "\nHome: %3"
                      "\nServer: %4")
         .arg( af::Environment::getCGRULocation().c_str())
         .arg( af::Environment::getAfRoot().c_str())
         .arg( af::Environment::getHomeAfanasy().c_str())
         .arg( af::Environment::getServerName().c_str());

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
    setToolTip( tooltip);

    connect( &timer, SIGNAL( timeout()), this, SLOT( refreshMessage() ));
    timer.setInterval( 32);
}

LabelVersion::~LabelVersion()
{
}

void LabelVersion::paintEvent( QPaintEvent * event)
{
    QPainter p( this);

    if( status != SS_None)
    {
        if( status == SS_Info )
            p.fillRect( rect(), afqt::QEnvironment::clr_Window.c);
        else
            p.fillRect( rect(), afqt::QEnvironment::clr_error.c);
        p.setPen( afqt::QEnvironment::qclr_black);
        p.setFont( font);

        static int offset = 0;
        static const int add_offset = 50;
        static int msg_width = 0;
        QRect border( 0, 0, 0, 0);
        offset = phase * 2;
        if( msg_width != 0)
            offset = offset % (msg_width + add_offset);
        int offset_cur = -offset;
        int counter = 0;
//printf("\noffset=%d width=%d\n", offset, rect().width());
        while( offset_cur <= rect().width())
        {
            p.drawText( rect().x() + offset_cur,
                     rect().y(),
                     rect().width() - border.x(),
                     rect().height(),
                     Qt::AlignVCenter | Qt::AlignLeft,
                     message,
                     &border);
            msg_width = border.width();
//printf("offset_cur=%d border.x=%d border.w=%d\n", offset_cur, border.x(), border.width());
            offset_cur += border.width() + add_offset;
            counter ++;
            if( counter > 20 ) break;
        }
        return;
    }

    // Fill area:
    //p.fillRect( rect(), afqt::QEnvironment::clr_Window.c);

/*   // Draw logo:
   if( false == logo.isNull())
      p.drawPixmap( 0, 0, logo);
*/
    // Draw text:
    p.setPen( afqt::QEnvironment::qclr_black);
    p.setFont( font);
    p.drawText( rect(), Qt::AlignCenter, label);
}

void LabelVersion::showMessage( const std::string & str)
{
    std::cout << str << std::endl;
    if(( status != SS_None ) && ( message == afqt::stoq(str) )) return;
    phase = 0;
    message = afqt::stoq( str);
    status = getStringStatus( str);
    if( status == SS_None ) return;
    setToolTip( message);
    repaint();
    timer.start();
}

void LabelVersion::refreshMessage()
{
    if( status == SS_None )
    {
        timer.stop();
        return;
    }
    phase++;
    repaint();
}

void LabelVersion::resetMessage()
{
    timer.stop();
    setToolTip( tooltip);
    status = SS_None;
    repaint();
    Watch::displayInfo( message);
}

void LabelVersion::mousePressEvent( QMouseEvent * event) { resetMessage();}

int LabelVersion::getStringStatus( const std::string & str)
{
    if( str.find("AFANASY") == 0 ) return SS_Info;
    if( str.find("ALARM")   == 0 ) return SS_Alarm;
    if( str.find("ACHTUNG") == 0 ) return SS_Alarm;
    return SS_None;
}

void LabelVersion::mouseDoubleClickEvent( QMouseEvent * event )
{
    WndText * wnd = new WndText("Info");
    wnd->insertText( label + "\n" + tooltip);
}
