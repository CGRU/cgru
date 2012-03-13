#pragma once

#include <QtGui/QWidget>

#include <name_af.h>

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
