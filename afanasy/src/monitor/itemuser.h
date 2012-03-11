#pragma once

#include "../libafanasy/user.h"

#include "itemnode.h"

class ItemUser : public ItemNode
{
public:
   ItemUser( af::User * user, QListWidget * parent = 0);
   ~ItemUser();

   void upNode( const af::Node * node);

private:
};
