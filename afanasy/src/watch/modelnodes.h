#pragma once

#include "../libafanasy/af.h"

#include "modelitems.h"

class Item;
class ItemNode;
class ListNodes;

class ModelNodes : public ModelItems
{
	Q_OBJECT

  public:
	ModelNodes(ListNodes *i_list_nodes);

	virtual ~ModelNodes();

	int addNode(ItemNode *node, int row = -1);

	int addNodeSorted(ItemNode *node);

	void sortnodes(const QList<ItemNode *> &itemstosort);

	void sortnodes();

	void sortMatch(const std::vector<int32_t> &i_list);

  signals:

	void nodeAdded(ItemNode *node, const QModelIndex &index);

  private:
};
