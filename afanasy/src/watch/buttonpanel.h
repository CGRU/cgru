#pragma once

#include "item.h"
#include "watch.h"

#include <QWidget>

class ButtonsMenu;
class ListItems;

class ButtonPanel : public QWidget
{
	Q_OBJECT

public:
	ButtonPanel(
		ListItems * i_listitems,
		Item::EType i_type,
		const QString & i_label,
		const QString & i_name,
		const QString & i_description,
		const QString & i_hotkey,
		bool i_dblclick,
		bool i_always_active,
		ButtonsMenu * i_bm);

	~ButtonPanel();

	static const int ms_Height;

	void keyPressed( const QString & i_str);

	static bool setHotkey( const QString & i_str);

	inline Item::EType getType() const {return m_type;}

	inline void setActive(bool i_active) {if(m_always_active) return; m_active = i_active; repaint();}

signals:
	void sigClicked();

protected:
	void paintEvent( QPaintEvent * i_evt);
	void enterEvent( QEvent * i_evt);
	void leaveEvent( QEvent * i_evt);
	void mousePressEvent( QMouseEvent * i_evt);
	void mouseDoubleClickEvent( QMouseEvent * i_evt);
	void contextMenuEvent( QContextMenuEvent * i_evt);

private:
	void clicked( QMouseEvent * i_evt, bool i_dbl);
	void emitSignal();
	void updateTip();

private slots:
	void deactivate();
	void listenHotkey();

private:
	Item::EType m_type;
	int m_height;
	bool m_active;
	QString m_label;
	QString m_name;
	QString m_description;
	QString m_hotkey;
	bool m_dblclick;
	bool m_always_active;

	bool m_hovered;
	bool m_activated;

	ListItems * m_listitems;

	ButtonsMenu * m_buttonsmenu;

	static ButtonPanel * ms_button_hotkey;
};
