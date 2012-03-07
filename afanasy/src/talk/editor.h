#pragma once

#include <QtGui/QTextEdit>

class Editor : public QTextEdit
{
Q_OBJECT
public:
   Editor( QWidget * parent);

signals:
   void activated();
   void sendMessage();

protected:
   void keyPressEvent (   QKeyEvent * event );
   void keyReleaseEvent ( QKeyEvent * event );
   void focusInEvent( QFocusEvent * event);

private:
   bool sendKeyPressed;
};
