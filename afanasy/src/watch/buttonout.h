#pragma once

#include "watch.h"

#include "../libafqt/qenvironment.h"

#include <QtGui/QWidget>

class ButtonOut : public QWidget
{
public:
    ButtonOut( const int i_button_position, QWidget * i_parent);
    ~ButtonOut();

    enum{ Center, Left, Right, LAST};

    inline static const int getType() { return ms_CurrentType; }

    void setSelected();
    void setUnSelected();

    void reloadImages();

protected:
    virtual void mousePressEvent( QMouseEvent * i_event );
    virtual void paintEvent( QPaintEvent * i_event);

private:
    bool m_selected;
    int  m_type;

    QPixmap m_img_off;
    QPixmap m_img_on;

private:
    static const int ms_width;
    static const int ms_height;

    static ButtonOut * ms_Buttons[LAST];
    static ButtonOut * ms_Current;
    static int ms_CurrentType;
};
