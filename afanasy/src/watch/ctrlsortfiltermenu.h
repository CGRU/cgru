#pragma once

#include "../libafqt/name_afqt.h"

#include <QLabel>

class CtrlSortFilterMenu : public QLabel
{
Q_OBJECT
public:
	CtrlSortFilterMenu( QWidget * i_parent, const int * i_checked_type);
	~CtrlSortFilterMenu();

	inline void addItem( int i_type) {m_types.push_back( i_type);}

signals:
	void sig_changed( int i_type);
	void sig_dblclick();

protected:
	void contextMenuEvent( QContextMenuEvent * i_event);
	void mouseDoubleClickEvent( QMouseEvent * i_event);

private slots:
	void slot_changed( int i_type);

private:
	std::vector<int> m_types;
	const int * m_checked_type;
};
