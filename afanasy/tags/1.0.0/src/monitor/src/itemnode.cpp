#include "itemnode.h"

//#define AFOUTPUT
////#undef AFOUTPUT
#include <macrooutput.h>

ItemNode::ItemNode( const af::Node * node, QListWidget * parent):
   QListWidgetItem( node->getName(), parent),
   id( node->getId())
{
//AFINFO("ItemNode::ItemNode:\n");
}

ItemNode::~ItemNode()
{
//AFINFO("ItemNode::~ItemNode:\n");
}
