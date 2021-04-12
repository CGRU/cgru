#pragma once

#include "../libafqt/qenvironment.h"

#include <QFrame>
#include <QLabel>

class Button;
class ListRenders;

class CtrlRenders : public QFrame
{
Q_OBJECT
public:
	CtrlRenders(QWidget * i_parent, ListRenders * i_listrenders);
	~CtrlRenders();

private slots:
	void slot_ThumsButtonClicked(Button*);

private:
	QList<Button*> m_thumbs_btns;
	static const QStringList ms_sizes_names;
	static const QList<int>  ms_sizes_enums;

	ListRenders * m_listrenders;
};

class CtrlRendersViewOptions : public QLabel
{
Q_OBJECT
public:
	CtrlRendersViewOptions(QWidget * i_parent, ListRenders * i_listrenders);
	~CtrlRendersViewOptions();

protected:
	void contextMenuEvent(QContextMenuEvent *event);

private:
	ListRenders * m_listrenders;
};
