#pragma once

#include "watch.h"

#include <QWidget>

class ButtonDblClick : public QWidget
{
	Q_OBJECT
public:
	ButtonDblClick( const QString & i_label, QWidget * i_parent);

	~ButtonDblClick();

	void setEnabled( bool i_enable);

signals:
	void sig_dblClicked();

protected:
	virtual void enterEvent( QEvent * i_evt);
	virtual void leaveEvent( QEvent * i_evt);
	virtual void paintEvent( QPaintEvent * i_evt);
	virtual void mouseDoubleClickEvent( QMouseEvent * i_evt);
	virtual void mouseReleaseEvent( QMouseEvent * i_evt);

private slots:
	void slot_SingleClickWarning();
	void slot_Clear();

private:
	QString m_label;
	QString m_text;

	int m_width;
	int m_height;

	bool m_enabled;
	bool m_hovered;
	bool m_clicked;
	bool m_activated;
	bool m_warning;
};

