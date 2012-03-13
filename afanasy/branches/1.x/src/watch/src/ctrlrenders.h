#pragma once

#include <QtGui/QLabel>

class QLabel;

class ListRenders;

class CtrlRenders : public QLabel
{
Q_OBJECT
public:
   CtrlRenders( QWidget * parent, ListRenders * renderslist);
   ~CtrlRenders();

//signals:

protected:
   void contextMenuEvent( QContextMenuEvent *event);

private slots:
   void actExpandTasks();

private:
   ListRenders * list;
};
