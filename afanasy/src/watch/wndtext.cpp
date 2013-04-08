#include "wndtext.h"

#include "../libafanasy/msg.h"

#include "../libafqt/name_afqt.h"

#include <QtGui/QLayout>
#include <QtGui/QTextEdit>

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
            qTextEdit->setPlainText( afqt::dtoq( msg->data(), msg->int32()));
            break;
         }
         case af::Msg::TString:
         {
            std::string str;
            msg->getString( str);
            if( str.size() == 0) str = "An empty string recieved.";
            qTextEdit->setPlainText( afqt::stoq( str));
            break;
         }
         case af::Msg::TStringList:
         {
            std::list<std::string> strlist;
            msg->getStringList( strlist);
            if( strlist.size() == 0) strlist.push_back("An empty list recieved.");
            for( std::list<std::string>::const_iterator it = strlist.begin(); it != strlist.end(); it++)
               qTextEdit->append( afqt::stoq(*it));
            break;
         }
         default:
            insertText("WndText::WndText: Invalid message:\n");
            insertText( afqt::stoq( msg->v_generateInfoString()));
      }
   }
}

WndText::~WndText()
{
}

void WndText::insertText( const QString text)
{
   QTextCursor c( qTextEdit->textCursor());
   c.insertText( text);
}
