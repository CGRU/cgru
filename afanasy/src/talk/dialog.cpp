#include "dialog.h"

#include <QtGui/QBoxLayout>
#include <QtGui/QApplication>
#include <QtCore/QEvent>
#include <QtGui/QCloseEvent>

#include "../libafanasy/environment.h"
#include "../libafanasy/talk.h"
#include "../libafanasy/msgclasses/mcafnodes.h"
#include "../libafanasy/msgclasses/mctalkmessage.h"
#include "../libafanasy/msgclasses/mctalkdistmessage.h"

#include "../libafqt/name_afqt.h"

#include "talkhost.h"
#include "textview.h"
#include "userslist.h"
#include "editor.h"
#include "tray.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Dialog::Dialog():
   connected(false),
   init(false )
{
   qServer = new afqt::QServer( this);
   qthreadClientUp = new afqt::QThreadClientUp( this, false, af::Environment::getTalkUpdatePeriod(), af::Environment::getTalkConnectRetries());
   qthreadClientSend = new afqt::QThreadClientSend( this);

   if( qServer->isInitialized() == false )
   {
      AFERROR("Dialog::Dialog: Server initialization failed.\n");
      return;
   }

   username = af::Environment::getUserName();
   hostname = af::Environment::getHostName();
   setWindowTitle((std::string("Talk::") + username + "@" + hostname + ":(connecting...)").c_str());

   editor = new Editor(this);
   textView  = new TextView(  this, afqt::stoq( username));
   usersList = new UsersList( this, afqt::stoq( username));

   QHBoxLayout *hlayout = new QHBoxLayout( this);
   QVBoxLayout *vlayout = new QVBoxLayout();
   hlayout->addLayout( vlayout);
   hlayout->addWidget( usersList);
   vlayout->addWidget( textView);
   vlayout->addWidget( editor);

   connect( qServer,         SIGNAL( newMsg( af::Msg*)), this, SLOT( caseMessage( af::Msg*)));
   connect( qthreadClientUp, SIGNAL( newMsg( af::Msg*)), this, SLOT( caseMessage( af::Msg*)));
   connect( qthreadClientUp, SIGNAL( connectionLost() ), this, SLOT( connectionLost()));

   connect( editor, SIGNAL( sendMessage()), this, SLOT( sendMessage()));

   connect( textView,  SIGNAL( activated()), this, SLOT( activated()));
   connect( usersList, SIGNAL( activated()), this, SLOT( activated()));
   connect( editor,    SIGNAL( activated()), this, SLOT( activated()));

   tray = new Tray( this, QString::fromUtf8( username.c_str()));
   connect( tray, SIGNAL( hideRaiseDialog()), this, SLOT( hideRaiseDialog()));

   talk = new TalkHost();
   init = true;
   talk->v_stdOut();

   sendRegister();
}

Dialog::~Dialog()
{
   qthreadClientSend->send( new af::Msg( af::Msg::TTalkDeregister, talk->getId()));
   if( talk != NULL) delete talk;
}

void Dialog::closeEvent(   QCloseEvent * event) { hide(); event->ignore();}
void Dialog::focusInEvent( QFocusEvent * event) {/*QWidget::focusInEvent( event);*/activated(); }
void Dialog::activated()   { tray->resetIcon(); }
void Dialog::hideEvent( QHideEvent * event) { windowRect = geometry(); }
void Dialog::showEvent( QShowEvent * event) { static const QRect empty; if( windowRect != empty) setGeometry(windowRect); }

void Dialog::hideRaiseDialog()
{
   if( isHidden()) show();
   else if( isActiveWindow()) hide();
   else
   {
      raise();
      setWindowState( Qt::WindowActive);
   }
}

void Dialog::connectionLost()
{
   if( connected == false ) return;
   connected = false;

   talk->setId( 0);
   sendRegister();
   printf("Dialog::connectionLost(): connecting...\n");

   usersList->clear();
   setWindowTitle( afqt::stoq((std::string("Talk::") + username + "@" + hostname + ":(connecting...)")));
}
void Dialog::sendRegister()
{
    qthreadClientUp->setUpMsg( new af::Msg( af::Msg::TTalkRegister, talk, true));
}

void Dialog::caseMessage( af::Msg * msg)
{
//AFINFO("void Dialog::caseMessage( Msg msg)\n");
   if( msg == NULL)
   {
      AFERROR("Dialog::caseMessage: msg == NULL\n");
      return;
   }
#ifdef AFOUTPUT
   msg->stdOut();
#endif
   switch( msg->type())
   {
   case af::Msg::TClientExitRequest:
   case af::Msg::TVersionMismatch:
   case af::Msg::TMagicMismatch:
   {
      emit stop();
      break;
   }
   case af::Msg::TTalkId:
   {
      if( talk->getId())
      {
         if( msg->int32() != talk->getId()) connectionLost();
      }
      else
      {
         if( msg->int32())
         {
            talk->setId( msg->int32());
            qthreadClientUp->setUpMsg( new af::Msg( af::Msg::TTalkUpdateId, talk->getId(), true));
         }
         else connectionLost();
      }
      break;
   }
   case af::Msg::TTalksList:
   {
      upOnline( *msg);
      break;
   }
   case af::Msg::TTalkData:
   {
      af::MCTalkmessage amsg( msg);
      std::string user, text;
      amsg.getUser( user);
      amsg.getText( text);
      appendMessage( afqt::stoq( user), afqt::stoq( text));
      break;
   }
   default:
      AFERROR("Dialog::caseMessage Unknown message recieved.\n");
      msg->v_stdOut();
   }
   delete msg;
}

void Dialog::upOnline( af::Msg &msg)
{
   if( !connected)
   {
      connected = true;
      std::string title = std::string("Talk::")+af::Environment::getUserName()+"@"+af::Environment::getHostName()+":";
      setWindowTitle( afqt::stoq( title) + QString::number( af::Environment::getClientPort()));
   }

   af::MCAfNodes mctalks( &msg);
   QStringList users;
   QStringList hosts;
   for( int i = 0; i < mctalks.getCount(); i++)
   {
      users << afqt::stoq( ((af::Talk*)mctalks.getNode(i))->getUserName());
      hosts << afqt::stoq( ((af::Talk*)mctalks.getNode(i))->getName());
   }
   usersList->updateOnline( users, hosts);
   tray->updateToolTip( users, hosts);
}

void Dialog::sendMessage()
{
   QString text = editor->toPlainText();
   if (text.isEmpty()) return;

   QList<QListWidgetItem*> qlist = usersList->selectedItems();
   int count_sel = qlist.count();
   if( count_sel < 1) return;

   af::MCTalkdistmessage mcdmsg( username, text.toUtf8().data());
   for( int sel = 0; sel < count_sel; sel++) mcdmsg.addUser( qlist[sel]->text().toUtf8().data());

   qthreadClientSend->send( new af::Msg( af::Msg::TTalkDistributeData, &mcdmsg));
   editor->clear();
}

void Dialog::appendMessage( const QString &from, const QString &message)
{
   if (from.isEmpty() || message.isEmpty()) return;

   textView->appendMessage( from, message);

   if( false == isVisible()) tray->showMessage( from, message);
   if( hasFocus()) return;
   if( editor->hasFocus()) return;
   if( textView->hasFocus()) return;
   if( usersList->hasFocus()) return;
   tray->highlightIcon();
}
