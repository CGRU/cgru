#include "editor.h"

#include <QtCore/QEvent>
#include <QtGui/QKeyEvent>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Editor::Editor( QWidget * parent):
   QTextEdit( parent),
   sendKeyPressed( false)
{
   setMaximumHeight( 100);
}

void Editor::keyPressEvent (   QKeyEvent * event )
{
   QTextEdit::keyPressEvent( event);
   if( event->key() == Qt::Key_Control) sendKeyPressed = true;
   if( sendKeyPressed && ((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return)))
      emit sendMessage();
}

void Editor::keyReleaseEvent ( QKeyEvent * event )
{
   QTextEdit::keyReleaseEvent( event);
   if( event->key() == Qt::Key_Control) sendKeyPressed = false;
}

void Editor::focusInEvent( QFocusEvent * event)
{
   QTextEdit::focusInEvent( event);
   emit activated();
}
