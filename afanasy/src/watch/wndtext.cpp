#include "wndtext.h"

#include "../libafanasy/logger.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/service.h"

#include "../libafqt/name_afqt.h"

#include <QLayout>
#include <QTextEdit>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

WndText::WndText( const QString & i_name):
	Wnd( i_name)
{
	construct();
}

WndText::WndText( const QString & i_name, af::Msg * i_msg):
	Wnd( i_name)
{
	construct();
	caseMsg( i_msg);
}

WndText::WndText( const QString & i_name, const std::string & i_str):
	Wnd( i_name)
{
	construct();
	insertText( i_str);
}

void WndText::construct()
{
	qTextEdit = new QTextEdit( this);

	//qTextEdit->setAcceptRichText( false);
	qTextEdit->setLineWrapMode( QTextEdit::NoWrap);
	qTextEdit->setReadOnly( true);

	layout = new QVBoxLayout( this);
#if QT_VERSION >= 0x040300
	layout->setContentsMargins( 1, 1, 1, 1);
#endif
	layout->addWidget( qTextEdit);
}

WndText::~WndText()
{
}

void WndText::caseMsg( af::Msg * i_msg)
{
	switch (i_msg->type())
	{
		case af::Msg::TDATA:
		{
			qTextEdit->setPlainText( afqt::dtoq( i_msg->data(), i_msg->int32()));
			break;
		}
		case af::Msg::TString:
		{
			std::string str;
			i_msg->getString( str);
			if( str.size() == 0) str = "An empty string recieved.";
			qTextEdit->setPlainText( afqt::stoq( str));
			break;
		}
		case af::Msg::TStringList:
		{
			std::list<std::string> strlist;
			i_msg->getStringList( strlist);
			if( strlist.size() == 0) strlist.push_back("An empty list recieved.");
			for( std::list<std::string>::const_iterator it = strlist.begin(); it != strlist.end(); it++)
			   qTextEdit->append( afqt::stoq(*it));
			break;
		}
		default:
			insertText("WndText::WndText: Invalid message:\n");
			insertText( i_msg->v_generateInfoString());
	}
}

void WndText::insertText( const char * i_text)
{
	QTextCursor c( qTextEdit->textCursor());
	c.insertText( QString::fromUtf8( i_text));
}

void WndText::insertText( const QString & i_text)
{
	QTextCursor c( qTextEdit->textCursor());
	c.insertText( i_text);
}

void WndText::insertText( const std::string & i_text)
{
	QTextCursor c( qTextEdit->textCursor());
	c.insertText( afqt::stoq( i_text));
}
