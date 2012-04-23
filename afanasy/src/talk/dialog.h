#pragma once

#include "../libafqt/qserver.h"
#include "../libafqt/qthreadclientsend.h"
#include "../libafqt/qthreadclientup.h"

#include <QtGui/QWidget>

class TalkHost;
class UsersList;
class TextView;
class Editor;
class Tray;

class Dialog : public QWidget
{
   Q_OBJECT

public:
   Dialog();
   ~Dialog();

   inline bool initialized() const { return init;}

signals:
   void stop();

protected:
   void closeEvent( QCloseEvent *event);
   void focusInEvent( QFocusEvent * event);
   void showEvent( QShowEvent * event);
   void hideEvent( QHideEvent * event);

private slots:
   void sendRegister();
   void caseMessage( af::Msg* msg);
   void connectionLost();
   void sendMessage();
   void activated();
   void hideRaiseDialog();

private:
   bool connected;

   std::string username;
   std::string hostname;
   TalkHost *talk;

   afqt::QThreadClientUp   *qthreadClientUp;
   afqt::QThreadClientSend *qthreadClientSend;
   afqt::QServer           *qServer;

   Editor *editor;
   TextView *textView;
   UsersList *usersList;

   Tray *tray;

   QRect windowRect;

   bool init;

private:
   void upOnline( af::Msg &msg);
   void appendMessage( const QString &from, const QString &message);
};
