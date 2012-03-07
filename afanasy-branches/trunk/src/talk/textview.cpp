#include "textview.h"

#include <QtCore/QDateTime>
#include <QtCore/QEvent>
#include <QtGui/QScrollBar>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

TextView::TextView( QWidget * parent, const QString & UserName):
   QTextEdit( parent),
   username( UserName)
{
   setReadOnly(true);
   formatTime.setFontItalic(true);
   formatName.setFontWeight(QFont::Bold);
}

TextView::~TextView()
{
}

void TextView::appendMessage( const QString &from, const QString &message)
{
   if (from.isEmpty() || message.isEmpty())
       return;

   QTextCursor cursor( textCursor());
   cursor.movePosition(QTextCursor::End);
   if( false == cursor.atBlockStart()) cursor.insertBlock();
   cursor.insertText( QDateTime::currentDateTime().toString("ddd h:mm"), formatTime);
   cursor.insertText( QString(" %1: ").arg( from), formatName);
   cursor.insertText( message, formatText);

   QScrollBar *bar = verticalScrollBar();
   bar->setValue( bar->maximum());
}
