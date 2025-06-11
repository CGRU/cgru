#include "modelnodes.h"

#include "item.h"
#include "itemnode.h"
#include "listnodes.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

ModelNodes::ModelNodes(ListNodes *i_list_nodes) : ModelItems(i_list_nodes) {}

ModelNodes::~ModelNodes() {}

int ModelNodes::addNode(ItemNode *node, int row)
{
	if (row == -1)
		row = items.size();
	else if (row > items.size())
	{
		AF_ERR << "ModelNodes::addNode: row > items.size() (" << row << " > " << items.size() << ")";
		row = items.size();
	}
	addItem(node, row);
	emit nodeAdded(node, index(row));
	return row;
}

int ModelNodes::addNodeSorted(ItemNode *node)
{
	int position = items.size();
	for (int i = 0; i < position; i++)
	{
		if (((ItemNode *)(items[i]))->compare(*node))
		{
			position = i;
			break;
		}
	}

	addNode(node, position);
	return position;
}

void ModelNodes::sortnodes(const QList<ItemNode *> &itemstosort)
{
	// printf("ModelNodes::sortnodes: \n");for( int s = 0; s < itemstosort.size(); s++) printf(" %s",
	// itemstosort[s]->getName().toUtf8().data()); printf("\n");

	emit layoutAboutToBeChanged();
	for (int s = 0; s < itemstosort.size(); s++)
	{
		ItemNode *item = itemstosort[s];
		// printf("ModelNodes::sortnodes: '%s'\n", item->getName().toUtf8().data());
		int row = -1;
		for (int i = 0; i < items.count(); i++)
			if (items[i] == item)
			{
				row = i;
				break;
			}
		if (row == -1)
		{
			AFERROR("ModelNodes::sortnodes: item not found.\n");
			return;
		}

		int pos = -1;
		int last = 0;
		for (int i = 0; i < items.count(); i++)
		{
			if (i == row)
			{
				last = i;
				continue;
			}

			// printf("ModelNodes::sortnodes: checking for '%s'[%d]:\n", items[i]->getName().toUtf8().data(),
			// i);
			bool sortingitem = false;
			for (int si = s + 1; si < itemstosort.size(); si++)
			{
				if (items[i] != itemstosort[si])
					continue;
				sortingitem = true;
				// printf("ModelNodes::sortnodes: '%s'[%d] is need to be sorted too.\n",
				// items[i]->getName().toUtf8().data(), i);
				break;
			}
			if (sortingitem)
				continue;

			last = i;
			if (((ItemNode *)(items[i]))->compare(*item))
			{
				pos = i;
				break;
			}
		}

		// printf("ModelNodes::sortnodes: row=%d pos=%d (last=%d)\n", row, pos, last);

		if (pos == -1)
			pos = last;
		else if (row < pos)
			pos--;

		if (row == pos)
			continue;

		if (row < pos)
			for (int i = row; i < pos; i++)
				items[i] = items[i + 1];
		else
			for (int i = row; i > pos; i--)
				items[i] = items[i - 1];
		items[pos] = item;
	}
	emit layoutChanged();
	// printf("ModelNodes::sortnodes: END.\n");
}

void ModelNodes::sortnodes()
{
	int numitems = items.size();
	if (numitems < 2)
		return;

	ItemNode **array = new ItemNode *[numitems];
	for (int i = 0; i < numitems; i++)
		array[i] = (ItemNode *)(items[i]);

	// The simplest (and slowest) method:
	for (int i = 0; i < numitems; i++)
	{
		for (int j = numitems - 1; j > i; j--)
		{
			ItemNode *item_a = array[j - 1];
			ItemNode *item_b = array[j];
			if (item_a->compare(*item_b))
			{
				array[j - 1] = item_b;
				array[j] = item_a;
			}
		}
	}

	emit layoutAboutToBeChanged();

	for (int i = 0; i < numitems; i++)
		items[i] = array[i];

	emit layoutChanged();

	delete[] array;
}

void ModelNodes::sortMatch(const std::vector<int32_t> &i_list)
{
	int numitems = items.size();
	if (numitems < 2)
		return;
	int listlen = int(i_list.size());
	if (listlen < 2)
		return;

	Item **array = new Item *[numitems];
	for (int i = 0; i < numitems; i++)
		array[i] = items[i];

	emit layoutAboutToBeChanged();

	int j = 0;
	for (int l = 0; l < listlen; l++)
	{
		for (int i = 0; i < numitems; i++)
		{
			if (array[i] == NULL)
				continue;
			if (array[i]->getId() == i_list[l])
			{
				items[j] = array[i];
				array[i] = NULL;
				j++;
			}
		}
		if (j >= numitems)
			break;
	}

	for (int i = 0; i < numitems; i++)
	{
		if (array[i] == NULL)
			continue;
		items[j] = array[i];
		j++;
		if (j >= numitems)
			break;
	}

	emit layoutChanged();

	delete[] array;
}
