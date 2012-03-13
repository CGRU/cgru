#include "itemrender.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ItemRender::ItemRender( af::Render * render, QListWidget * parent):
   ItemNode( render, parent)
{
   upNode( render);
}

ItemRender::~ItemRender()
{
}

void ItemRender::upNode( const af::Node * node)
{
   af::Render * render = (af::Render*)node;
   QString info = QString("%1-%2").arg( render->getName()).arg( render->getPriority());
   if( render->isBusy() ) info += QString(" Busy");
   setText( info);
}
