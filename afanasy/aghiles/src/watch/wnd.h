#pragma once

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
