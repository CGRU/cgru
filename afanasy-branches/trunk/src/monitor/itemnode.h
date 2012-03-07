#pragma once

#include <QtGui/QListWidget>

#include "../libafanasy/afnode.h"

class ItemNode : public QListWidgetItem
{
public:
   ItemNode( const af::Node * node, QListWidget * parent = 0);
   virtual ~ItemNode();

   inline int getId() const { return id; }

   virtual void upNode( const af::Node * node) = 0;

private:
   int id;
};
