#pragma once

#include "../libafqt/qenvironment.h"

#include <QtGui/QWidget>

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
