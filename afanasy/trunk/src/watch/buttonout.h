#pragma once

#include <QtGui/QWidget>

class ButtonOut : public QWidget
{
public:
    ButtonOut( const int button_position, QWidget *parent);
    ~ButtonOut();

    enum{ Center, Left, Right, LAST};

    inline static const int getType() { return CurrentType; }

    void setSelected();
    void setUnSelected();

    static const int width;
    static const int height;

protected:
    virtual void mousePressEvent( QMouseEvent * event );
    virtual void paintEvent( QPaintEvent * event);

private:
    bool selected;
    int type;

private:
    static ButtonOut * buttons[LAST];
    static ButtonOut * Current;
    static int CurrentType;
};
