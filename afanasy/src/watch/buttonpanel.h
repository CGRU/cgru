#pragma once

#include "watch.h"

#include <QWidget>

class ListItems;

class ButtonPanel : public QWidget
{
	Q_OBJECT

public:
	ButtonPanel(
		ListItems * i_listitems,
		const QString & i_label,
		const QString & i_name,
		const QString & i_description,
		const QString & i_hotkey = "",
		bool i_dblclick = false);

	~ButtonPanel();

	static const int ms_Width;
	static const int ms_Height;

	void keyPressed( const QString & i_str);

	static bool setHotkey( const QString & i_str);

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
	QString m_label;
	QString m_name;
	QString m_description;
	QString m_hotkey;
	bool m_dblclick;

	bool m_hovered;
	bool m_activated;

	ListItems * m_listitems;

	static ButtonPanel * ms_button_hotkey;
};
