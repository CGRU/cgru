#include "wndlistenjob.h"

#include <QtGui/QLayout>

#include <msg.h>
#include <environment.h>
#include <msgclasses/mclistenaddress.h>
#include <msgclasses/mctaskoutput.h>

#include <qmsg.h>

#include "watch.h"

//#define AFOUTPUT
////#undef AFOUTPUT
#include <macrooutput.h>

WndListenJob::WndListenJob( int JobId, const QString & Name):
   WndText( "Listen Job"),
   jobid( JobId),
   jobname( Name)
{
   layout->setContentsMargins( 10, 10, 10, 10);
   setContentsMargins(1,1,1,1);

   qTextEdit->moveCursor( QTextCursor::End, QTextCursor::KeepAnchor);
   QTextCursor cursor = qTextEdit->textCursor();
   QTextBlockFormat blockFormat;
   blockFormat.setTopMargin(-5);
   blockFormat.setBottomMargin(-5);
   cursor.setBlockFormat( blockFormat);
   qTextEdit->setTextCursor( cursor);
   qTextEdit->setWordWrapMode( QTextOption::NoWrap);
   setWindowTitle( jobname);

   af::MCListenAddress mclass( af::MCListenAddress::TOLISTEN, af::Environment::getAddress(), jobid);
   Watch::sendMsg( new afqt::QMsg( af::Msg::TTaskListenOutput, &mclass));
}

WndListenJob::~WndListenJob()
{
   Watch::listenJob_rem( jobid);
}

void WndListenJob::closeEvent( QCloseEvent * event)
{
   af::MCListenAddress mclass( 0, af::Environment::getAddress(), jobid);
   Watch::sendMsg( new afqt::QMsg( af::Msg::TTaskListenOutput, &mclass));
   Wnd::closeEvent( event);
}

bool WndListenJob::caseMessage( af::Msg * msg)
{
//printf("MonOutput::caseMessage:\n");msg.stdOutData();
   switch( msg->type())
   {
   case af::Msg::TTaskOutput:
   {
      af::MCTaskOutput mclass( msg);
//mclass.stdOut( true );
      if( mclass.getJobId() != jobid ) return false;
      qTextEdit->append( mclass.getRenderName() + ": " + QString::fromUtf8( mclass.getData(), mclass.getDataSize()));
      break;
   }
   default:
      return false;
   }
   return true;
}

void WndListenJob::connectionLost()
{
   close();
}
