#pragma once

#include <QtGui/QWidget>

class Popup : public QWidget
{
public:
    Popup( const QString & i_title, const QString & i_msg);
    ~Popup();

protected:
	virtual void mousePressEvent( QMouseEvent * event);
};
