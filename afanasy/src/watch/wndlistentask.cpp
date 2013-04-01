#include "wndlistentask.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mclistenaddress.h"
#include "../libafanasy/msgclasses/mctaskoutput.h"

#include "watch.h"

#include <QtGui/QLayout>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

WndListenTask::WndListenTask( int JobId, int BlockNum, int TaskNum, const QString & Name):
   WndText("Listen Task"),
   jobid(      JobId    ),
   block(      BlockNum ),
   task(       TaskNum  ),
   taskname(   Name     )
{
#if QT_VERSION >= 0x040300
   layout->setContentsMargins( 5, 5, 5, 5);
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
   setWindowTitle( taskname);
   taskname += " %1:";

   af::MCListenAddress mclass( af::MCListenAddress::JUSTTASK | af::MCListenAddress::TOLISTEN, Watch::getClientAddress(), jobid, block, task);
   Watch::sendMsg( new af::Msg( af::Msg::TTaskListenOutput, &mclass));
}

WndListenTask::~WndListenTask()
{
}

void WndListenTask::closeEvent( QCloseEvent * event)
{
   af::MCListenAddress mclass( af::MCListenAddress::JUSTTASK, Watch::getClientAddress(), jobid, block, task);
   Watch::sendMsg( new af::Msg( af::Msg::TTaskListenOutput, &mclass));
   Wnd::closeEvent( event);
}

bool WndListenTask::caseMessage( af::Msg * msg)
{
//printf("MonOutput::caseMessage:\n");msg.stdOutData();
   switch( msg->type())
   {
   case af::Msg::TTaskOutput:
   {
      af::MCTaskOutput mclass( msg);
//mclass.stdOut( true );
      if(( mclass.getJobId()     != jobid ) ||
         ( mclass.getNumBlock()  != block ) ||
         ( mclass.getNumTask()   != task  )) return false;
      qTextEdit->append( afqt::dtoq( mclass.getData(), mclass.getDataSize()));
      setWindowTitle( taskname.arg( afqt::stoq( mclass.getRenderName())));
      break;
   }
   default:
      return false;
   }
   return true;
}

void WndListenTask::v_connectionLost()
{
   close();
}
