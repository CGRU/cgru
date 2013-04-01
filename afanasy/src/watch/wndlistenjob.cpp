#include "wndlistenjob.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mclistenaddress.h"
#include "../libafanasy/msgclasses/mctaskoutput.h"

#include "watch.h"

#include <QtGui/QLayout>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

WndListenJob::WndListenJob( int JobId, const QString & Name):
   WndText( "Listen Job"),
   jobid( JobId),
   jobname( Name)
{
#if QT_VERSION >= 0x040300
   layout->setContentsMargins( 10, 10, 10, 10);
#endif
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

   af::MCListenAddress mclass( af::MCListenAddress::TOLISTEN, Watch::getClientAddress(), jobid);
   Watch::sendMsg( new af::Msg( af::Msg::TTaskListenOutput, &mclass));
}

WndListenJob::~WndListenJob()
{
   Watch::listenJob_rem( jobid);
}

void WndListenJob::closeEvent( QCloseEvent * event)
{
   af::MCListenAddress mclass( 0, Watch::getClientAddress(), jobid);
   Watch::sendMsg( new af::Msg( af::Msg::TTaskListenOutput, &mclass));
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
      qTextEdit->append( afqt::stoq( mclass.getRenderName()) + ": " + afqt::dtoq( mclass.getData(), mclass.getDataSize()));
      break;
   }
   default:
      return false;
   }
   return true;
}

void WndListenJob::v_connectionLost()
{
   close();
}
