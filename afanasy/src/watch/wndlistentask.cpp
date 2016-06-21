#include "wndlistentask.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "monitorhost.h"
#include "watch.h"

#include <QLayout>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

WndListenTask::WndListenTask( int i_job_id, int i_block, int i_task, const QString & i_name):
   WndText("Listen Task"),
   m_job_id(i_job_id),
   m_block(i_block),
   m_task(i_task),
   m_name(i_name)
{
#if QT_VERSION >= 0x040300
   layout->setContentsMargins( 5, 5, 5, 5);
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

	m_name += " %1:";

	subscribe( true);
}

WndListenTask::~WndListenTask()
{
}

void WndListenTask::closeEvent( QCloseEvent * event)
{
	subscribe( false);

   Wnd::closeEvent( event);
}

void WndListenTask::subscribe( bool i_subscribe)
{
	std::ostringstream str;
	af::jsonActionOperationStart( str,"monitors","watch","", MonitorHost::ids());
	str << ",\"class\":\"listen\"";
	str << ",\"job\":" << m_job_id;
	str << ",\"block\":" << m_block;
	str << ",\"task\":" << m_task;
	str << ",\"status\":\"" << ( i_subscribe ? "subscribe":"unsubscribe") << "\"";
	af::jsonActionOperationFinish( str);

	Watch::sendMsg( af::jsonMsg( str));
}

bool WndListenTask::v_processEvents( const af::MonitorEvents & i_me)
{
/*	for( int i = 0; i < i_me.m_listens.size(); i++)
	{
		if( ( i_me.m_listens[i].job_id == m_job_id ) &&
			( i_me.m_listens[i].block  == m_block  ) &&
			( i_me.m_listens[i].task   == m_task   ))
		{
			qTextEdit->append( afqt::stoq( i_me.m_listens[i].output));
			setWindowTitle( m_name.arg( afqt::stoq( i_me.m_listens[i].hostname)));

			return true;
		}
	}
*/
	return false;
}

void WndListenTask::v_connectionLost()
{
   close();
}
