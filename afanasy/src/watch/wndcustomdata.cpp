#include "wndcustomdata.h"

#include "../libafanasy/name_af.h"

#include <QLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

WndCustomData::WndCustomData( const QString & i_name, const QString & i_text):
	Wnd( i_name),
	m_text( i_text)
{

	QVBoxLayout * qVLayout = new QVBoxLayout( this);

	m_qText = new QPlainTextEdit( this);
	qVLayout->addWidget( m_qText);
	m_qText->setLineWrapMode( QPlainTextEdit::NoWrap);
	m_qText->setPlainText( m_text);

	QHBoxLayout * qHLayout = new QHBoxLayout();
	qVLayout->addLayout( qHLayout);

	QPushButton * qBCancel = new QPushButton("Cancel", this);
	qHLayout->addWidget( qBCancel);
	connect( qBCancel, SIGNAL( clicked()), this, SLOT( close()));

	QPushButton * qBCheck = new QPushButton("Check", this);
	qHLayout->addWidget( qBCheck);
	connect( qBCheck, SIGNAL( clicked()), this, SLOT( checkText()));

	QPushButton * qBSet = new QPushButton("Set", this);
	qHLayout->addWidget( qBSet);
	connect( qBSet, SIGNAL( clicked()), this, SLOT( setText()));

	m_qLine = new QLineEdit( this);
	qVLayout->addWidget( m_qLine);
	m_qLine->setReadOnly( true);
}

WndCustomData::~WndCustomData()
{
}

void WndCustomData::checkText()
{
	m_valid = false;
	m_newText = m_qText->toPlainText();
	std::string text = afqt::qtos( m_newText);

	rapidjson::Document doc;
	std::string msg;
	char * data = af::jsonParseData( doc, text.c_str(), text.size(), &msg);

	if( data == NULL )
	{
		msg = "ERROR at " + af::itos( doc.GetErrorOffset()) + ": " + doc.GetParseError();
		m_qLine->setText( afqt::stoq( msg));
		printf("%s\n", msg.c_str());
		return;
	}

	m_valid = true;
	m_qLine->setText("OK.");

	delete [] data;
}

void WndCustomData::setText()
{
	checkText();

	if( false == m_valid )
		return;

	if( m_text == m_newText )
	{
		m_qLine->setText("Text was not changed.");
		return;
	}

	emit textEdited( m_newText);

	close();
}

