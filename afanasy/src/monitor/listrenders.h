#pragma once

#include "listnodes.h"

class ListRenders : public ListNodes
{
public:
   ListRenders( QWidget * parent);
   ~ListRenders();

   bool caseMessage( af::Msg* msg);

   ItemNode * createNode( af::Node * node);

private:
};
