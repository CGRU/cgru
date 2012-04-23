#pragma once

#include "../libafqt/qenvironment.h"

#include <QtGui/QLabel>

class ListRenders;

class CtrlRenders : public QLabel
{
Q_OBJECT
public:
   CtrlRenders( QWidget * i_parent, ListRenders * i_renderslist);
   ~CtrlRenders();

protected:
   void contextMenuEvent( QContextMenuEvent *event);

private:
   ListRenders * m_list;
};
