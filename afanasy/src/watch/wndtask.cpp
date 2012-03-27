#include "wndtask.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QTextEdit>

#include "../libafanasy/msg.h"
#include "../libafanasy/service.h"
#include "../libafanasy/taskexec.h"

#include "../libafqt/name_afqt.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

WndTask::WndTask( const QString & Name, af::Msg * msg):
	Wnd( Name)
{
	QVBoxLayout * layout = new QVBoxLayout( this);
	#if QT_VERSION >= 0x040300
	layout->setContentsMargins( 1, 1, 1, 1);
	#endif

	QTextEdit * qTextEdit = new QTextEdit( this);
	layout->addWidget( qTextEdit);
	qTextEdit->setAcceptRichText( false);
	qTextEdit->setLineWrapMode( QTextEdit::NoWrap);
	qTextEdit->setReadOnly( true);

	// Check message type:
	if( msg->type() != af::Msg::TTask )
	{
		QTextCursor c( qTextEdit->textCursor());
		c.insertText("WndTask::WndTask: Message is not of type 'Task:':\n");
		c.insertText( afqt::stoq( msg->generateInfoString()));
		return;
	}

	// Create task and service:
	af::TaskExec taskexec( msg);
	af::Service service( taskexec);
	QString wdir      = afqt::stoq( service.getWDir());
	QString command   = afqt::stoq( service.getCommand());
	QString files     = afqt::stoq( service.getFiles());

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
	if( files.isEmpty() == false)
	{
		c.insertText( "\n");
		c.insertText( "Files:", fInfo);
		c.insertText( "\n");
		c.insertText( files.replace(";","\n"), fParameter);
	}
	if( taskexec.hasFileSizeCheck())
	{
		c.insertText( "\n");
		c.insertText( "File Size Check: ", fInfo);
		c.insertText( QString::number( taskexec.getFileSizeMin()), fParameter);
		c.insertText( " - ", fInfo);
		c.insertText( QString::number( taskexec.getFileSizeMax()), fParameter);
	}

	if( files.isEmpty())
		return;

	// Create preview files controls:
	layout->addWidget( new QLabel("Preview Files:", this));
	QStringList filesList = files.split(';');
	for( int i = 0; i < filesList.size(); i++)
	{
		layout->addWidget( new QLabel( filesList[i], this));
	}
}

WndTask::~WndTask()
{
}
