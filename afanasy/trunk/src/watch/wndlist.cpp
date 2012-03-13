#include "wndlist.h"

#include <QtGui/QLayout>

#include "../libafanasy/msg.h"

#include "watch.h"
#include "listitems.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

WndList::WndList( const QString & Name, int monType ):
   Wnd( Name),
   type( monType)
{
   layout = new QVBoxLayout( this);
   layout->setContentsMargins( 1, 1, 1, 1);

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
