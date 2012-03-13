#include "wndtext.h"

#include <QtGui/QLayout>
#include <QtGui/QTextEdit>

#include "../libafanasy/msg.h"
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
   layout->setContentsMargins( 1, 1, 1, 1);
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
            QTextCharFormat fParameter;
            QTextCharFormat fInfo;
            fParameter.setFontWeight(QFont::Bold);
            fInfo.setFontItalic(true);

            QTextCursor c( qTextEdit->textCursor());

            af::TaskExec task( msg);
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
            c.insertText( task.getCmd(), fParameter);
            c.insertText( "\n");
            c.insertText( "Working Directory:", fInfo);
            c.insertText( "\n");
            c.insertText( task.getWDir(), fParameter);
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
