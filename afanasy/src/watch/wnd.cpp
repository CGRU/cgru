#include "wnd.h"

#include "buttonout.h"
#include "dialog.h"
#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Wnd::Wnd( const QString & Name):
   QWidget( Watch::getDialog()),
   name( Name)
{
    setWindowTitle( name);
    setWindowFlags( Qt::Window);
    setAttribute (  Qt::WA_DeleteOnClose, true );

    QRect rect( Watch::getDialog()->geometry());
    switch( ButtonOut::getType())
    {
        case ButtonOut::Center:
        {
            rect.translate( 30, 30);
            afqt::QEnvironment::getRect( name, rect);
            break;
        }
        case ButtonOut::Left:
        {
            rect.translate( -rect.width() , 0);
            break;
        }
        case ButtonOut::Right:
        {
            rect.translate( +rect.width(), 0);
            break;
        }
    }

    setGeometry( rect);

    show();

    Watch::addWindow( this);

AFINFO("Wnd::Wnd:\n");
}

Wnd::~Wnd()
{
AFINFO("Wnd::~Wnd:\n");
    Watch::removeWindow( this);
}

void Wnd::closeEvent( QCloseEvent * event)
{
AFINFA("Wnd::closeEvent: '%s' = %d, %d, %d, %d\n", name.toUtf8().data(), geometry().x(), geometry().y(), geometry().width(), geometry().height());
    afqt::QEnvironment::setRect( name, geometry());
}
