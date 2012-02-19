#include "itemnode.h"

#include "../libafqt/name_afqt.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ItemNode::ItemNode( const af::Node * node, QListWidget * parent):
   QListWidgetItem( afqt::stoq( node->getName()), parent),
   id( node->getId())
{
//AFINFO("ItemNode::ItemNode:\n");
}

ItemNode::~ItemNode()
{
//AFINFO("ItemNode::~ItemNode:\n");
}
