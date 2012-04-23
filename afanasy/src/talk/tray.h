#pragma once

#include "../libafanasy/environment.h"

#include "../libafqt/name_afqt.h"

#include <QtGui/QSystemTrayIcon>

class QTimer;

class Tray : public QSystemTrayIcon
{
Q_OBJECT
public:
   Tray( QWidget * parent, const QString & UserName);
   ~Tray();

   void resetIcon();
   void highlightIcon();

   void updateToolTip( const QStringList & users, const QStringList & hosts);

signals:
   void hideRaiseDialog();

private slots:
   void trayActivated( QSystemTrayIcon::ActivationReason reason);
   void changeIcon();

private:
   QIcon icon;
   QIcon icon_msg;
   bool  icon_flashing;
   bool  icon_state;

   QTimer * timer;

   QString username;
   QString trayToolTipHeader;
};
