#pragma once

#include "../libafanasy/name_af.h"

#include "wnd.h"

#include <QtGui/QPushButton>

class QContextMenuEvent;

class ButtonMenu : public QPushButton
{
Q_OBJECT
public:
	ButtonMenu( const QString & i_file, const QString & i_wdir, QWidget * i_parent);

public slots:
	void pushed_slot();
	void launchCmd( int i_index);

protected:
	void contextMenuEvent( QContextMenuEvent * i_event);

private:
	QString m_file;
	QString m_wdir;
	QStringList m_labels;
	QStringList m_cmds;
};

class WndTask : public Wnd
{
public:
	WndTask( const QString & Name, af::Msg * msg = NULL);
	~WndTask();
};
