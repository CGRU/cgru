#include "wndlistenjob.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "monitorhost.h"
#include "watch.h"

#include <QLayout>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

WndListenJob::WndListenJob( int i_job_id, const QString & i_name):
   WndText( "Listen Job"),
	m_job_id( i_job_id),
	m_name( i_name)
{
#if QT_VERSION >= 0x040300
   layout->setContentsMargins( 10, 10, 10, 10);
#endif
   setContentsMargins(1,1,1,1);

   qTextEdit->moveCursor( QTextCursor::End, QTextCursor::KeepAnchor);
   QTextCursor cursor = qTextEdit->textCursor();
   QTextBlockFormat blockFormat;
//   blockFormat.setTopMargin(-5);
//   blockFormat.setBottomMargin(-5);
   cursor.setBlockFormat( blockFormat);
   qTextEdit->setTextCursor( cursor);
   qTextEdit->setWordWrapMode( QTextOption::NoWrap);

	m_name = QString("Listening \"%1\"").arg( m_name);

	setWindowTitle( m_name);

	subscribe( true);
}

WndListenJob::~WndListenJob()
{
	Watch::listenJob_rem( m_job_id);
}

void WndListenJob::closeEvent( QCloseEvent * event)
{
	subscribe( false);
}

void WndListenJob::subscribe( bool i_subscribe)
{
	std::ostringstream str;
	af::jsonActionOperationStart( str,"monitors","watch","", MonitorHost::ids());
	str << ",\"class\":\"listen\"";
	str << ",\"job\":" << m_job_id;
	str << ",\"status\":\"" << ( i_subscribe ? "subscribe":"unsubscribe") << "\"";
	af::jsonActionOperationFinish( str);

	Watch::sendMsg( af::jsonMsg( str));
}

bool WndListenJob::v_processEvents( const af::MonitorEvents & i_me)
{
	bool founded = false;

	for( int i = 0; i < i_me.m_listens.size(); i++)
	{
		if( i_me.m_listens[i].getJobId() == m_job_id )
		{
			qTextEdit->setFontWeight( QFont::Bold);
			qTextEdit->setFontItalic( true);
			qTextEdit->setFontUnderline( true);
			qTextEdit->append( afqt::stoq( i_me.m_listens[i].m_task_name)
					+ "[" + afqt::stoq( i_me.m_listens[i].m_progress.hostname) + "]:");

			qTextEdit->setFontUnderline( false);
			qTextEdit->setFontItalic( false);
			qTextEdit->setFontWeight( QFont::Normal);
			qTextEdit->append( afqt::stoq( i_me.m_listens[i].getListened()));

			founded = true;
		}
	}

	return founded;
}

void WndListenJob::v_connectionLost()
{
   close();
}

