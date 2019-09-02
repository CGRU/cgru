#pragma once

#include "watch.h"

#include <QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>

class ButtonPanel;
class ListItems;

class ButtonsMenu : public QWidget
{
	Q_OBJECT

public:
	ButtonsMenu(ListItems * i_listitems, const QString & i_label, const QString & i_tip);

	~ButtonsMenu();

	void addButton(ButtonPanel * i_bp);

	void openMenu();
	void closeMenu();

signals:
	void sigClicked();

protected:
	void paintEvent(QPaintEvent * i_evt);
	void enterEvent(QEvent * i_evt);
	void leaveEvent(QEvent * i_evt);
	void mousePressEvent(QMouseEvent * i_evt);

private:
	void updateLabel();
	void clicked(QMouseEvent * i_evt, bool i_dbl);

private:
	QVBoxLayout * m_layout;
	QLabel * m_qlabel;

	QString m_label;

	bool m_hovered;
	bool m_opened;

	std::vector<ButtonPanel*> m_buttons;

	ListItems * m_listitems;
};
