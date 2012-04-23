#include "wndlist.h"

#include "../libafanasy/msg.h"

#include "watch.h"
#include "listitems.h"

#include <QtGui/QLayout>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

WndList::WndList( const QString & Name, int monType ):
   Wnd( Name),
   type( monType)
{
   layout = new QVBoxLayout( this);
#if QT_VERSION >= 0x040300
   layout->setContentsMargins( 1, 1, 1, 1);
#endif

   Watch::opened[type] = this;
}

WndList::~WndList()
{
   Watch::opened[type] = NULL;
}

void WndList::setList( ListItems * list)
{
   itemslist = list;
   layout->addWidget( itemslist);
}

void WndList::repaintItems()
{
   itemslist ->repaintItems();
}
