#pragma once

#include "listnodes.h"

class ListUsers : public ListNodes
{
public:
   ListUsers( QWidget * parent);
   ~ListUsers();

   bool caseMessage( af::Msg* msg);

   ItemNode * createNode( af::Node * node);

private:
};
