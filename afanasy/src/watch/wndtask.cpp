#include "wndtask.h"

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/service.h"
#include "../libafanasy/taskexec.h"

#include "../libafqt/name_afqt.h"

#include "actionid.h"
#include "watch.h"

#include <QtGui/QContextMenuEvent>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>
#include <QTextEdit>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ButtonMenu::ButtonMenu( const QString & i_file, const QString & i_wdir, QWidget * i_parent):
	QPushButton("Launch", i_parent),
	m_file( i_file),
	m_wdir( i_wdir)
{
	std::vector<std::string>::const_iterator it = af::Environment::getPreviewCmds().begin();
	for( ; it != af::Environment::getPreviewCmds().end(); it++)
	{
		QString cmd( afqt::stoq(*it));
		m_labels.append( cmd);
		cmd = cmd.replace( AFWATCH::CMDS_ARGUMENT, m_file);
		m_cmds.append( cmd);
	}

	connect( this, SIGNAL( pressed()), this, SLOT( pushed_slot()));
}

void ButtonMenu::pushed_slot()
{
	contextMenuEvent( new QContextMenuEvent( QContextMenuEvent::Mouse, QPoint()));
}

void ButtonMenu::contextMenuEvent( QContextMenuEvent * i_event)
{
	QMenu menu(this);
	ActionId * action;

	action = new ActionId( -1, "Browse", this);
	connect( action, SIGNAL( triggeredId( int) ), this, SLOT( launchCmd( int)));
	menu.addAction( action);

	menu.addSeparator();

	for( int i = 0; i < m_cmds.size(); i++)
	{
		action = new ActionId( i, m_labels[i], this);
		connect( action, SIGNAL( triggeredId( int) ), this, SLOT( launchCmd( int)));
		menu.addAction( action);
	}

	menu.exec( i_event->globalPos());
}

void ButtonMenu::launchCmd( int i_index)
{
	if( i_index == -1 )
	{
		Watch::browseImages( m_file, m_wdir);
		return;
	}

	if( i_index >= m_cmds.size())
	{
		AFERRAR("ButtonMenu::launchCmd: index is out of range: %d >= %d", i_index, m_cmds.size())
		return;
	}
	
	Watch::startProcess( m_cmds[i_index], m_wdir);
}

WndTask::WndTask( const QString & Name, af::Msg * msg):
	Wnd( Name)
{
	QVBoxLayout * box = new QVBoxLayout( this);

	QSplitter * splitter = new QSplitter( Qt::Vertical, this);
	box->addWidget( splitter);

	QTextEdit * qTextEdit = new QTextEdit( this);
	splitter->addWidget( qTextEdit);
	qTextEdit->setAcceptRichText( false);
	qTextEdit->setLineWrapMode( QTextEdit::NoWrap);
	qTextEdit->setReadOnly( true);

	// Check message type:
	if( msg->type() != af::Msg::TTask )
	{
		QTextCursor c( qTextEdit->textCursor());
		c.insertText("WndTask::WndTask: Message is not of type 'Task:':\n");
		c.insertText( afqt::stoq( msg->v_generateInfoString()));
		return;
	}

	// Create task and service:
	af::TaskExec taskexec( msg);
	af::Service service( &taskexec);
	QString wdir    = afqt::stoq( service.getWDir());
	QString command = afqt::stoq( service.getCommand());
	std::vector<std::string> files = service.getFiles();
	std::vector<std::string> parsed_files = service.getParsedFiles();

	// Output task data:
	QTextCharFormat fParameter;
	QTextCharFormat fInfo;
	fParameter.setFontWeight(QFont::Bold);
	fInfo.setFontItalic(true);

	QTextCursor c( qTextEdit->textCursor());

	c.insertText( afqt::stoq( taskexec.getName()), fParameter);
	c.insertText( "\n");
	c.insertText( afqt::stoq( taskexec.getServiceType()), fParameter);
	c.insertText( "[", fInfo);
	c.insertText( afqt::stoq( taskexec.getParserType()), fParameter);
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
	if( files.size())
	{
		c.insertText( "\n");
		c.insertText( "Files:", fInfo);
		for( int i = 0; i < files.size(); i++)
			c.insertText( "\n" + afqt::stoq( files[i]), fParameter);
	}
/*	if( parsed_files.size())
	{
		c.insertText( "\n");
		c.insertText( "Parsed Files:", fInfo);
		for( int i = 0; i < parsed_files.size(); i++)
			c.insertText( "\n" + afqt::stoq( parsed_files[i]), fParameter);
	}*/
	if( taskexec.hasFileSizeCheck())
	{
		c.insertText( "\n");
		c.insertText( "File Size Check: ", fInfo);
		c.insertText( QString::number( taskexec.getFileSizeMin()), fParameter);
		c.insertText( " - ", fInfo);
		c.insertText( QString::number( taskexec.getFileSizeMax()), fParameter);
	}

	if( parsed_files.size())
		files = parsed_files;

	if( files.empty())
		return;


	// Create preview files controls:
	QScrollArea * scroll = new QScrollArea();
//	scroll->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn);
//	scroll->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn);
	QWidget * widget = new QWidget();
	QVBoxLayout * layout = new QVBoxLayout();
	widget->setLayout( layout);
	scroll->setWidget( widget);
	splitter->addWidget( scroll);

	splitter->setSizes( QList<int>() << 300 << ( files.size() > 10 ? 300 : files.size() * 30 ));
	scroll->setWidgetResizable( true);
	layout->setSpacing(0);
	#if QT_VERSION >= 0x040300
	layout->setContentsMargins( 1, 1, 1, 1);
	#endif

	if( parsed_files.size())
		layout->addWidget( new QLabel("Preview Files (parsed):", this));
	else
		layout->addWidget( new QLabel("Preview Files:", this));
	QHBoxLayout * layoutH = new QHBoxLayout();
	QVBoxLayout * layoutL = new QVBoxLayout();
	QVBoxLayout * layoutR = new QVBoxLayout();
	layout ->addLayout( layoutH);
	layoutH->addLayout( layoutL);
	layoutH->addLayout( layoutR);

	for( int i = 0; i < files.size(); i++)
	{
		QLineEdit * fileField = new QLineEdit( this);
		layoutL->addWidget( fileField);
		fileField->setReadOnly(true);
		fileField->setText( afqt::stoq( files[i]));

		ButtonMenu * btnBrowse = new ButtonMenu( afqt::stoq( files[i]), wdir, this);
		layoutR->addWidget( btnBrowse);
	}
}

WndTask::~WndTask()
{
}
