#pragma once

#include "item.h"

#include <QtCore/QAbstractItemModel>

class Item;
class ListItems;

class ModelItems : public QAbstractListModel
{
  public:
	ModelItems(ListItems *i_list_items);
	virtual ~ModelItems();

	int rowCount(const QModelIndex &) const;
	inline int count() const { return items.size(); }
	QVariant data(const QModelIndex &index, int role) const;

	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

	void addItem(Item *item, int row = -1);

	void delItem(int row);

	void deleteAllItems();

	void emit_dataChanged(int firstChangedRow = -1, int lastChangedRow = -1);

	inline Item *item(int row) { return items[row]; }

	inline int getRow(Item *item) const { return items.indexOf(item); }

	void deleteZeroItems();

	inline void itemsHeightChanged() { layoutChanged(); }

	void itemsHeightCalc();

	void setItems(int start, Item **item, int count);

  protected:
	QList<Item *> items;

  private:
	ListItems *m_list_items;
};
