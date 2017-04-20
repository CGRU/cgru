#pragma once

#include "../libafqt/qenvironment.h"

#include <QWidget>

class Wnd : public QWidget
{
public:
    Wnd( const QString & Name);
    ~Wnd();

protected:
    virtual void closeEvent( QCloseEvent * event);

private:
    QString name;
};
