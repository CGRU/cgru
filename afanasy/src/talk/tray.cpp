#include "tray.h"

#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QMenu>
#include <QtCore/QTimer>
#include <QtGui/QWidget>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Tray::Tray( QWidget * parent, const QString & UserName):
   QSystemTrayIcon( parent),
   icon( afqt::stoq( af::Environment::getAfRoot()) + "/icons/talk_tray.png"),
   icon_msg( afqt::stoq( af::Environment::getAfRoot()) + "/icons/talk_tray_msg.png"),
   icon_flashing( false),
   icon_state( false),
   username( UserName),
   trayToolTipHeader("afTalk users:")
{
   connect( this, SIGNAL(     activated( QSystemTrayIcon::ActivationReason )),
            this, SLOT(   trayActivated( QSystemTrayIcon::ActivationReason )));

   QAction * restoreAction = new QAction(tr("&Hide|Raise"), this);
   connect(  restoreAction,  SIGNAL( triggered()), parent, SLOT( hideRaiseDialog()));
   QAction * quitAction    = new QAction(tr("&Quit" ), this);
   connect(  quitAction,     SIGNAL( triggered()), qApp,   SLOT( quit()));

   QMenu *trayIconMenu = new QMenu( parent);
   trayIconMenu->addAction( restoreAction);
   trayIconMenu->addSeparator();
   trayIconMenu->addAction( quitAction);
   setContextMenu(trayIconMenu);

   timer = new QTimer( this);
   timer->setInterval( 500);
   connect( timer, SIGNAL( timeout()), this, SLOT( changeIcon()));

   setIcon( icon);
   setToolTip("af Talk");
   show();
}

Tray::~Tray()
{
}

void Tray::trayActivated( QSystemTrayIcon::ActivationReason reason)
{
   if( reason == QSystemTrayIcon::DoubleClick) emit hideRaiseDialog();
}

void Tray::resetIcon()
{
   timer->stop();
   setIcon( icon);
   icon_state = false;
   icon_flashing = false;
}

void Tray::highlightIcon()
{
   setIcon( icon_msg);
   icon_state = true;
   icon_flashing = true;
   timer->start();
}

void Tray::changeIcon()
{
   if( icon_state || ( false == icon_flashing ))
   {
      setIcon( icon);
      icon_state = false;
   }
   else
   {
      setIcon( icon_msg);
      icon_state = true;
   }
}

void Tray::updateToolTip( const QStringList & users, const QStringList & hosts)
{
   QString toolTip( trayToolTipHeader);
   QStringList usernames;

   for( int i = 0; i < users.size(); i++)
      if( users[i] == username )
         toolTip += QString("\n%1").arg( hosts[i]);
      else if( false == usernames.contains( users[i]))
         usernames << users[i];

   for( int i = 0; i < usernames.size(); i++) toolTip += QString("\n%1").arg( usernames[i]);

   setToolTip( toolTip);
}
