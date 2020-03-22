#pragma once

#include "actionid.h"

#include <QLabel>

class ListWork;

class CtrlWork : public QLabel
{
Q_OBJECT
public:
	CtrlWork( QWidget * i_parent, ListWork * i_listwork);
	~CtrlWork();

protected:
	void contextMenuEvent( QContextMenuEvent * i_event);

private:
	ListWork * m_listwork;
};
