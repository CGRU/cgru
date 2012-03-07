#pragma once

#include "../libafanasy/render.h"

#include "itemnode.h"

class ItemRender : public ItemNode
{
public:
   ItemRender( af::Render * render, QListWidget * parent = 0);
   ~ItemRender();

   void upNode( const af::Node * node);

private:
};
