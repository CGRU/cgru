#pragma once

#include "../libafqt/name_afqt.h"

#include <QWidget>

class CheckBoxWidget : public QWidget
{
Q_OBJECT
public:
    CheckBoxWidget( QWidget * parent, afqt::AttrNumber * attrNumber);
    ~CheckBoxWidget();

private slots:
    void valueChanged( int value);

private:
    afqt::AttrNumber * num;
};
