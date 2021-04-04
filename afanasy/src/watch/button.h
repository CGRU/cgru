#pragma once

#include "../libafqt/name_afqt.h"

#include <QWidget>

class Button : public QWidget
{
	Q_OBJECT

public:
	Button(
		const QString & i_label,
		const QString & i_name = QString(),
		const QString & i_tooltip = QString(),
		bool i_dblclick = false,
		bool i_radio = false,
		int  i_clicked_ms = 500);

	~Button();

	inline const QString & getName() const {return m_name;}

	inline void setActive( bool i_active ) {if(i_active  == m_active ) return; m_active  = i_active;  repaint();}
	inline void setEnabled(bool i_enabled) {if(i_enabled == m_enabled) return; m_enabled = i_enabled; repaint();}

signals:
	void sig_Clicked(Button*);

protected:
	void paintEvent(QPaintEvent * i_evt);
	void enterEvent(QEvent * i_evt);
	void leaveEvent(QEvent * i_evt);
	void mousePressEvent(QMouseEvent * i_evt);
	void mouseDoubleClickEvent(QMouseEvent * i_evt);

private:
	void clicked(QMouseEvent * i_evt, bool i_dbl);
	void emitSignal();

private slots:
	void slot_ClickedFinished();

private:
	bool m_enabled;
	bool m_radio;
	bool m_active;
	QString m_label;
	QString m_name;
	QString m_tooltip;
	bool m_dblclick;

	bool m_hovered;
	bool m_clicked;
	int  m_clicked_ms;
};
