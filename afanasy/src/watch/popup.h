#pragma once

#include "watch.h"

#include <QWidget>

class Popup : public QWidget
{
public:
    Popup( const QString & i_title, const QString & i_msg, uint32_t i_state = 0);
    ~Popup();

protected:
	virtual void mousePressEvent( QMouseEvent * event);
	virtual void closeEvent( QCloseEvent * event);

private:
	static Popup * ms_popup;
};
