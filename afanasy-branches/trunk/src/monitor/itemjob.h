#pragma once

#include "../libafanasy/job.h"

#include "itemnode.h"

class ItemJob : public ItemNode
{
public:
   ItemJob( af::Job * job, QListWidget * parent = 0);
   ~ItemJob();

   void upNode( const af::Node * node);

private:
};
