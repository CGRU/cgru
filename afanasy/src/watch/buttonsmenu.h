#pragma once

#include "item.h"
#include "watch.h"

#include <QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>

class ButtonPanel;
class ListItems;

class ButtonsMenuLabel : public QWidget
{
public:
	ButtonsMenuLabel(const QString & i_text, QWidget * i_parent = NULL);
	~ButtonsMenuLabel();
	inline void setOpened() {m_opened = true;  repaint();}
	inline void setClosed() {m_opened = false; repaint();}
	inline void setActive(bool i_active) {m_active = i_active; repaint();}
	void setHidden(bool i_hide);
protected:
	void paintEvent(QPaintEvent * i_evt);
private:
	QString m_text;
	bool m_opened;
	bool m_active;
	bool m_hidden;
};

class ButtonsMenu : public QWidget
{
	Q_OBJECT

public:
	ButtonsMenu(ListItems * i_listitems, Item::EType i_type, const QString & i_label, const QString & i_tip);

	~ButtonsMenu();

	void addButton(ButtonPanel * i_bp);

	void openMenu();
	void closeMenu();
	inline void setActive(bool i_active) {m_active = i_active; m_label->setActive(m_active); repaint();}

	inline Item::EType getType() const {return m_type;}

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
	ButtonsMenuLabel * m_label;
	QWidget * m_btns_widget;
	QVBoxLayout * m_btns_layout;

	Item::EType m_type;

	bool m_hovered;
	bool m_opened;
	bool m_active;

	std::vector<ButtonPanel*> m_buttons;

	ListItems * m_listitems;
};
