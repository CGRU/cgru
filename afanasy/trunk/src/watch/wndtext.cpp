#include "wndtext.h"

#include <QtGui/QLayout>
#include <QtGui/QTextEdit>

#include "../libafanasy/msg.h"
#include "../libafanasy/service.h"
#include "../libafanasy/taskexec.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

WndText::WndText( const QString & Name, af::Msg * msg):
   Wnd( Name)
{
   qTextEdit = new QTextEdit( this);

   qTextEdit->setAcceptRichText( false);
   qTextEdit->setLineWrapMode( QTextEdit::NoWrap);
   qTextEdit->setReadOnly( true);

   layout = new QVBoxLayout( this);
#if QT_VERSION >= 0x040300
   layout->setContentsMargins( 1, 1, 1, 1);
#endif
   layout->addWidget( qTextEdit);

   if( msg )
   {
      switch (msg->type())
      {
         case af::Msg::TDATA:
         {
            qTextEdit->setPlainText( QString::fromUtf8( msg->data(), msg->int32()));
            break;
         }
         case af::Msg::TQString:
         {
            QString str;
            msg->getString( str);
            qTextEdit->setPlainText( str);
            break;
         }
         case af::Msg::TQStringList:
         {
            QStringList strlist;
            msg->getStringList( strlist);
            int size = strlist.size();
            for( int i = 0; i < size; i++)
               qTextEdit->append( strlist[i]);
            break;
         }
         case af::Msg::TTask:
         {
            showTask( msg);
            break;
         }
         default:
            AFERROR("WndText::WndText: Invalid message:\n");
            msg->stdOut();
      }
   }
}

WndText::~WndText()
{
}

void WndText::showTask( af::Msg * msg)
{
   af::TaskExec task( msg);
   QString wdir = task.getWDir();
   QString command = task.getCmd();
   af::Service * service;
   if( false == task.getServiceType().isEmpty())
   {
      service = new af::Service(
            task.getServiceType(),
            wdir,
            command,
            task.getCapCoeff(),
            task.getMultiHostsNames(),
            task.getCmdView()
         );
      if(( service != NULL ) && ( false == service->isInitialized() ))
      {
         delete service;
         service = NULL;
      }
   }
   if( service)
   {
      wdir = service->getWDir();
      command = service->getCommand();
   }

   QTextCharFormat fParameter;
   QTextCharFormat fInfo;
   fParameter.setFontWeight(QFont::Bold);
   fInfo.setFontItalic(true);

   QTextCursor c( qTextEdit->textCursor());

   c.insertText( task.getName(), fParameter);
   c.insertText( "\n");
   c.insertText( task.getServiceType(), fParameter);
   c.insertText( "[", fInfo);
   c.insertText( task.getParserType(), fParameter);
   c.insertText( "]:", fInfo);
   c.insertText( QString::number(task.getCapacity()), fParameter);
   c.insertText( " frames(", fInfo);
   c.insertText( QString::number(task.getFrameStart()), fParameter);
   c.insertText( ",", fInfo);
   c.insertText( QString::number(task.getFrameFinish()), fParameter);
   c.insertText( ",", fInfo);
   c.insertText( QString::number(task.getFramesNum()), fParameter);
   c.insertText( "):", fInfo);
   c.insertText( "\n");
   c.insertText( "Command:", fInfo);
   c.insertText( "\n");
   c.insertText( command, fParameter);
   c.insertText( "\n");
   c.insertText( "Working Directory:", fInfo);
   c.insertText( "\n");
   c.insertText( wdir, fParameter);
   if( task.getCmdView().isEmpty() == false)
   {
      c.insertText( "\n");
      c.insertText( "Preview:", fInfo);
      c.insertText( "\n");
      c.insertText( task.getCmdView(), fParameter);
   }
   if( task.hasFileSizeCheck())
   {
      c.insertText( "\n");
      c.insertText( "File Size Check: ", fInfo);
      c.insertText( QString::number( task.getFileSizeMin()), fParameter);
      c.insertText( " - ", fInfo);
      c.insertText( QString::number( task.getFileSizeMax()), fParameter);
   }

   if( service != NULL) delete service;
}
