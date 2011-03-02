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
         case af::Msg::TString:
         {
            std::string str;
            msg->getString( str);
            if( str.size() == 0) str = "An empty string recieved.";
            qTextEdit->setPlainText( QString::fromUtf8( str.c_str()));
            break;
         }
         case af::Msg::TStringList:
         {
            std::list<std::string> strlist;
            msg->getStringList( strlist);
            if( strlist.size() == 0) strlist.push_back("An empty list recieved.");
            int size = strlist.size();
            for( std::list<std::string>::const_iterator it = strlist.begin(); it != strlist.end(); it++)
               qTextEdit->append( QString::fromUtf8((*it).c_str()));
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
   af::TaskExec taskexec( msg);
   af::Service service( taskexec);
   QString wdir = service.getWDir();
   QString command = service.getCommand();
   QString files = service.getFiles();

   QTextCharFormat fParameter;
   QTextCharFormat fInfo;
   fParameter.setFontWeight(QFont::Bold);
   fInfo.setFontItalic(true);

   QTextCursor c( qTextEdit->textCursor());

   c.insertText( taskexec.getName(), fParameter);
   c.insertText( "\n");
   c.insertText( taskexec.getServiceType(), fParameter);
   c.insertText( "[", fInfo);
   c.insertText( taskexec.getParserType(), fParameter);
   c.insertText( "]:", fInfo);
   c.insertText( QString::number( taskexec.getCapacity()), fParameter);
   c.insertText( " frames(", fInfo);
   c.insertText( QString::number( taskexec.getFrameStart()), fParameter);
   c.insertText( ",", fInfo);
   c.insertText( QString::number( taskexec.getFrameFinish()), fParameter);
   c.insertText( ",", fInfo);
   c.insertText( QString::number( taskexec.getFramesNum()), fParameter);
   c.insertText( "):", fInfo);
   c.insertText( "\n");
   c.insertText( "Command:", fInfo);
   c.insertText( "\n");
   c.insertText( command, fParameter);
   c.insertText( "\n");
   c.insertText( "Working Directory:", fInfo);
   c.insertText( "\n");
   c.insertText( wdir, fParameter);
   if( files.isEmpty() == false)
   {
      c.insertText( "\n");
      c.insertText( "Preview:", fInfo);
      c.insertText( "\n");
      c.insertText( files, fParameter);
   }
   if( taskexec.hasFileSizeCheck())
   {
      c.insertText( "\n");
      c.insertText( "File Size Check: ", fInfo);
      c.insertText( QString::number( taskexec.getFileSizeMin()), fParameter);
      c.insertText( " - ", fInfo);
      c.insertText( QString::number( taskexec.getFileSizeMax()), fParameter);
   }
}
